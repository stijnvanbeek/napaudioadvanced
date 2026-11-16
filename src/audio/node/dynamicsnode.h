/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Audio includes
#include <audio/core/audionode.h>

#include <audio/utility/linearsmoothedvalue.h>

namespace nap
{
	namespace audio
	{

		/**
		 * Standard dynamic range compressor, with an optional limiter mode.
		 *
		 * The input signal level is measured with a fast/slow envelope follower.
		 * Whenever the measured level exceeds the threshold the gain is reduced
		 * according to the compression ratio. The amount of reduction is smoothed
		 * with separate attack and release time constants and is finally applied
		 * to the signal (optionally with automatic makeup gain).
		 *
		 * The transfer characteristic follows the classic piecewise-linear law:
		 *   x <= threshold            :  y = x
		 *   x >  threshold            :  y = threshold + (x - threshold) / ratio
		 *
		 * In @ref Mode::Limiter the node drives its gain from the envelope
		 * follower to keep the measured level at or below the threshold (acting
		 * as an infinite-ratio limiter) and applies a per-sample safety clamp so
		 * that no transient can push the output above the ceiling even when the
		 * envelope lags. A soft knee of configurable width (see @ref setKnee)
		 * starts the gain reduction below the ceiling and approaches the ceiling
		 * exponentially without ever crossing it, so limiting begins before the
		 * ceiling is reached while the output stays below it at all times. The
		 * attack and release times still shape the gain response; the ratio
		 * parameter is not used in this mode.
		 *
		 * All parameters are exposed in human-readable units (dB, seconds) and
		 * can be changed at any time, even while the node is running.
		 */
		class NAPAPI DynamicsNode : public Node
		{
			RTTI_ENABLE(Node)
		public:
			/**
			 * Operating mode.
			 */
			enum class Mode
			{
				Compressor,    ///< Standard dynamic compression (finite ratio, smoothed gain).
				Limiter        ///< Infinite-ratio ceiling clamp with sample-accurate peak detection.
			};

			static constexpr float smoothTime = 50.f;

		public:
			explicit DynamicsNode(NodeManager& nodeManager);

			InputPin  input  = { this };     ///< Audio input pin
			OutputPin output = { this };     ///< Audio output pin

			/**
			 * Sets the operating mode.
			 * @param mode Compressor (standard) or Limiter (ceiling clamp).
			 */
			void setMode(Mode mode);

			/**
			 * @return the current operating mode.
			 */
			Mode getMode() const { return mMode.load(); }

			/**
			 * Sets the compression threshold.
			 * @param threshold Threshold in dB (e.g. -18.0).
			 */
			void setThreshold(ControllerValue threshold);

			/**
			 * @return the threshold in dB
			 */
			ControllerValue getThreshold() const { return mThreshold.load(); }

			/**
			 * Sets the compression ratio.
			 * @param ratio Ratio, 1.0 (bypass) up to e.g. 20.0.
			 */
			void setRatio(ControllerValue ratio);

			/**
			 * @return the compression ratio
			 */
			ControllerValue getRatio() const { return mRatio.load(); }

			/**
			 * Sets the soft-knee width for limiter mode.
			 * @param knee Knee width in dB. Gain reduction starts knee below the
			 * ceiling and rises smoothly while the output approaches the ceiling
			 * exponentially without ever exceeding it. 0 disables the knee (hard
			 * knee exactly at the ceiling).
			 */
			void setKnee(ControllerValue knee);

			/**
			 * @return the soft-knee width in dB
			 */
			ControllerValue getKnee() const { return mKnee.load(); }

			/**
			 * Sets the attack time.
			 * @param attack Time in milliseconds it takes the gain to reduce when the signal exceeds the threshold.
			 */
			void setAttack(ControllerValue attack);

			/**
			 * @return the attack time in milliseconds
			 */
			ControllerValue getAttack() const { return mAttack.load(); }

			/**
			 * Sets the release time.
			 * @param release Time in milliseconds it takes the gain to return to unity after the signal falls below the threshold.
			 */
			void setRelease(ControllerValue release);

			/**
			 * @return the release time in milliseconds
			 */
			ControllerValue getRelease() const { return mRelease.load(); }

			/**
			 * Enables or disables automatic makeup gain, which compensates for
			 * the average amount of gain reduction.
			 * @param enabled true to apply makeup gain.
			 */
			void setMakeupGainEnabled(bool enabled) { mMakeupGainEnabled.store(enabled); }

			/**
			 * @return true when automatic makeup gain is enabled.
			 */
			bool getMakeupGainEnabled() const { return mMakeupGainEnabled.load(); }

			/**
			 * Bypasses the node: the input is passed through to the output without any
			 * processing.
			 * @param bypass true to bypass the node.
			 */
			void setByPass(bool bypass);

			/**
			 * @return true when the node is bypassed.
			 */
			bool isBypassed() const { return mBypass.load(); }

			/**
			 * @return the current (instantaneous) gain reduction in dB.
			 */
			ControllerValue getGainReduction() const { return mGainReductionDB; }

		private:
			void process() override;

			void sampleRateChanged(float sampleRate) override;

			/// Recomputes the attack/release smoothing coefficients from the current time values and sample rate.
			void updateCoefficients();

			std::atomic<Mode>            mMode           = { Mode::Compressor };
			std::atomic<ControllerValue> mThreshold      = { -18.f };    ///< Threshold in dB (ceiling in limiter mode)
			std::atomic<ControllerValue> mRatio          = { 4.f  };     ///< Compression ratio
			std::atomic<ControllerValue> mAttack         = { 10.f   };  ///< Attack time in milliseconds
			std::atomic<ControllerValue> mRelease        = { 200.f  };  ///< Release time in milliseconds
			std::atomic<ControllerValue> mKnee           = { 3.f    };  ///< Soft-knee width in dB (limiter mode)
			std::atomic<bool>            mMakeupGainEnabled = { true };
			std::atomic<bool>            mBypass          = { false };

			// Envelope follower state (audio thread only)
			ControllerValue mEnvelope = 0.f;                          ///< Current measured level (linear)
			ControllerValue mGain = 0.f;                               ///< Current applied gain reduction (linear)
			ControllerValue mGainReductionDB = 0.f;                    ///< Current gain reduction in dB (for metering)
			ControllerValue mMakeupGain = 0.f;                         ///< Slowly tracked makeup gain in dB (0 = no makeup), both modes

			// Smoothing coefficients, updated when parameters or the sample rate change
			ControllerValue mAttackCoeff  = 0.f;
			ControllerValue mReleaseCoeff = 0.f;
			ControllerValue mMakeupCoeff  = 0.f;

			LinearSmoothedValue<ControllerValue> mDryWet = { 1.f, 64 }; // Used internally for smooth bypassing. 0 = dry, 1 = wet.
		};

	}

}
