/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Std includes
#include <atomic>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/utility/dirtyflag.h>
#include <audio/utility/linearsmoothedvalue.h>

namespace nap
{
	namespace audio
	{

		/**
		 * Multi-purpose DSP filter using a topology-preserving (TPT/ZDF) state-variable algorithm,
		 * after Andrew Simper's trapezoidal SVF.
		 *
		 * Unlike FilterNode's bilinear biquad, the cutoff and resonance parameters are smoothed
		 * per sample and the coefficients are recomputed from the ramping parameters, so every
		 * intermediate state is a valid, stable filter. Together with the preserved integrator
		 * state this makes the node suitable for fast, even audio-rate modulation of the cutoff
		 * frequency and resonance without zipper noise, instability or clicks.
		 *
		 * Applies lowpass, highpass, bandpass, notch and peak filtering.
		 */
		class NAPAPI SVFNode : public Node
		{
			RTTI_ENABLE(Node)

		public:
			enum class EMode
			{
				LowPass, HighPass, BandPass, Notch, Peak
			};

		public:
			SVFNode(NodeManager& nodeManager);

			/**
			 * The input to be filtered
			 */
			InputPin audioInput = { this };

			/**
			 * Outputs the filtered signal
			 */
			OutputPin audioOutput = { this };

			/**
			 * Immediately changes the settings of the filter.
			 * Only call this before the filter starts being processed.
			 * @param mode lowpass, highpass, bandpass, notch or peak filtering.
			 * @param frequency The cutoff (lowpass, highpass) or center (bandpass, notch, peak) frequency in Hz.
			 * @param resonance The resonance of the filter. 0 is the flat Butterworth response,
			 * higher values add a resonance peak with an exponentially rising Q, 30 approaches
			 * self-oscillation.
			 * @param gain Sets the gaining factor of the filter's output.
			 */
			void prepare(EMode mode, ControllerValue frequency, ControllerValue resonance, ControllerValue gain);

			/**
			 * Sets the mode of the filter.
			 * @param mode lowpass, highpass, bandpass, notch or peak filtering.
			 */
			void setMode(EMode mode);

			/**
			 * Sets the frequency parameter of the filter in Hz. For the different modes this means:
			 * - Cutoff frequency for lowpass and highpass filtering
			 * - Center frequency for bandpass, notch and peak filtering
			 */
			void setFrequency(ControllerValue frequency);

			/**
			 * Sets the resonance of the filter, using the same convention as FilterNode.
			 * 0 is the flat Butterworth response, higher values add a resonance peak with an
			 * exponentially rising Q, 30 approaches self-oscillation. The value is clamped
			 * between 0 and 30.
			 */
			void setResonance(ControllerValue resonance);

			/**
			 * Sets the gaining factor of the filter's output.
			 */
			void setGain(ControllerValue gain);

			/**
			 * Sets all the filter parameters at once.
			 * @param mode lowpass, highpass, bandpass, notch or peak filtering.
			 * @param frequency The cutoff or center frequency in Hz.
			 * @param resonance The resonance of the filter.
			 * @param gain Sets the gaining factor of the filter's output.
			 */
			void set(EMode mode, ControllerValue frequency, ControllerValue resonance, ControllerValue gain);

			/**
			 * Sets the smoothing time for the frequency, resonance and gain parameters to fade to their next value.
			 * The coefficients are recalculated per sample from the ramping parameters while they fade.
			 * @param smoothing Time in ms.
			 */
			void setSmoothing(TimeValue smoothing);

			/**
			 * @return the mode of the filter.
			 */
			EMode getMode() const { return mMode; }

			/**
			 * @return the cutoff frequency (for lowpass and highpass filters) or the center frequency (for the other modes) in Hz.
			 */
			ControllerValue getFrequency() const { return mFrequency; }

			/**
			 * @return the resonance of the filter.
			 */
			ControllerValue getResonance() const { return mResonance; }

			/**
			 * @return the output gain factor of the filter.
			 */
			ControllerValue getGain() const { return mGain; }

		private:
			// Inherited from Node
			void process() override;
			void sampleRateChanged(float sampleRate) override;

			void calcCoeffs(ControllerValue frequency, ControllerValue resonance);

			std::atomic<EMode> mMode = { EMode::LowPass };
			std::atomic<ControllerValue> mFrequency = { 440.f };
			std::atomic<ControllerValue> mResonance = { 0.f };
			std::atomic<ControllerValue> mGain = { 1.f };
			DirtyFlag mIsDirty;

			// The parameters are ramped per sample and the coefficients are recalculated from
			// the ramping values, so every intermediate filter is a stable, valid SVF.
			LinearSmoothedValue<ControllerValue> mFrequencyValue = { 440.f, 64 };
			LinearSmoothedValue<ControllerValue> mResonanceValue = { 0.f, 64 };
			LinearSmoothedValue<ControllerValue> mGainValue = { 1.f, 64 };
			TimeValue mSmoothing = 1.f;

			// Coefficients of the trapezoidal integrator form:
			// g is the cutoff term (tan of the scaled cutoff frequency), k the damping mapped
			// from the resonance parameter exactly as FilterNode maps it, and a1, a2 and a3
			// the precomputed integrator factors.
			ControllerValue g = 0;
			ControllerValue k = 0;
			ControllerValue a1 = 0;
			ControllerValue a2 = 0;
			ControllerValue a3 = 0;
			ControllerValue mLastFrequency = -1;
			ControllerValue mLastResonance = -1;
			bool mRecalcCoeffs = true;

			// Integrator state, preserved across parameter changes (topology-preserving).
			ControllerValue mIc1 = 0; // Band pass integrator state
			ControllerValue mIc2 = 0; // Low pass integrator state
		};

	}
}
