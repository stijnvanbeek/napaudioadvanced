/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "DynamicsNode.h"

#include <audio/core/audionodemanager.h>
#include <audio/utility/audiofunctions.h>

#include <algorithm>

#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::DynamicsNode)
	RTTI_FUNCTION("setMode", &nap::audio::DynamicsNode::setMode)
	RTTI_FUNCTION("getMode", &nap::audio::DynamicsNode::getMode)
	RTTI_FUNCTION("setThreshold", &nap::audio::DynamicsNode::setThreshold)
	RTTI_FUNCTION("setRatio", &nap::audio::DynamicsNode::setRatio)
	RTTI_FUNCTION("setKnee", &nap::audio::DynamicsNode::setKnee)
	RTTI_FUNCTION("getKnee", &nap::audio::DynamicsNode::getKnee)
	RTTI_FUNCTION("setAttack", &nap::audio::DynamicsNode::setAttack)
	RTTI_FUNCTION("setRelease", &nap::audio::DynamicsNode::setRelease)
	RTTI_FUNCTION("setMakeupGainEnabled", &nap::audio::DynamicsNode::setMakeupGainEnabled)
	RTTI_FUNCTION("getGainReduction", &nap::audio::DynamicsNode::getGainReduction)
RTTI_END_CLASS

namespace nap
{

	namespace audio
	{

		DynamicsNode::DynamicsNode(NodeManager& nodeManager)
			: Node(nodeManager)
		{
			updateCoefficients();
			mDryWet.setStepCount(nodeManager.getSamplesPerMillisecond() * smoothTime);
		}


		void DynamicsNode::setMode(Mode mode)
		{
			mMode.store(mode);
			// Reset the tracked gain state so the mode switch does not cause a click.
			mGain = 1.f;
			// Makeup gain is tracked in dB in both modes (0.f = no makeup).
			mMakeupGain = 0.f;
		}


		void DynamicsNode::setThreshold(ControllerValue threshold)
		{
			mThreshold.store(threshold);
		}


		void DynamicsNode::setRatio(ControllerValue ratio)
		{
			if (ratio < 1.f)
				ratio = 1.f;
			mRatio.store(ratio);
		}


		void DynamicsNode::setKnee(ControllerValue knee)
		{
			if (knee < 0.f)
				knee = 0.f;
			mKnee.store(knee);
		}


		void DynamicsNode::setAttack(ControllerValue attack)
		{
			if (attack < 0.f)
				attack = 0.f;
			mAttack.store(attack);
			updateCoefficients();
		}


		void DynamicsNode::setRelease(ControllerValue release)
		{
			if (release < 0.f)
				release = 0.f;
			mRelease.store(release);
			updateCoefficients();
		}


		void DynamicsNode::sampleRateChanged(float sampleRate)
		{
			updateCoefficients();
			mDryWet.setStepCount(getNodeManager().getSamplesPerMillisecond() * smoothTime);
		}


		void DynamicsNode::updateCoefficients()
		{
			auto sampleRate = getNodeManager().getSampleRate();
			if (sampleRate <= 0.f)
				return;

			// Convert a time constant (milliseconds) into a per-sample one-pole smoothing coefficient.
			// Smaller time constant -> larger coefficient -> faster response.
			auto timeToCoeff = [sampleRate](ControllerValue time) {
				if (time <= 0.f)
					return 1.f; // Immediate response
				return 1.f - expf(-1000.f / (time * sampleRate));
			};

			mAttackCoeff = timeToCoeff(mAttack.load());
			mReleaseCoeff = timeToCoeff(mRelease.load());
			// Makeup gain tracks the average reduction on a slow time scale.
			mMakeupCoeff = timeToCoeff(200.f);
		}


		void DynamicsNode::setByPass(bool bypass)
		{
			if (bypass == mBypass.load())
				return;

			mBypass.store(bypass);
			if (bypass)
				mDryWet.setValue(0.f);
			else
				mDryWet.setValue(1.f);
		}

		void DynamicsNode::process()
		{
			auto& outputBuffer = getOutputBuffer(output);
			auto* inputBuffer = input.pull();

			if (inputBuffer == nullptr)
			{
				std::fill(outputBuffer.begin(), outputBuffer.end(), 0.f);
				return;
			}

			// Bypassed: pass the input through to the output without any processing.
			if (mBypass.load() && !mDryWet.isRamping())
			{
				std::copy(inputBuffer->begin(), inputBuffer->end(), outputBuffer.begin());
				return;
			}

			const auto mode = mMode.load();

			if (mode == Mode::Limiter)
			{
				// ---- Limiter: envelope-driven gain + sample-accurate safety clamp ----
				// The main gain is driven by the fast/slow envelope follower so the
				// response is smooth and musical (same measurement as the compressor
				// branch). A per-sample clamp acts as a safety net so that no transient
				// can push the output above the ceiling even when the envelope lags.
				auto ceiling = dbToA(mThreshold.load(), -120.f);
				if (ceiling <= 0.f)
					ceiling = 1.f; // threshold at/below silence -> no limiting
				const auto ceilingDB = mThreshold.load();

				auto makeupEnabled = mMakeupGainEnabled.load();

				for (auto i = 0; i < outputBuffer.size(); ++i)
				{
					auto in = (*inputBuffer)[i];
					auto level = std::fabs(in);

					// 1. Track the input level with the fast/slow envelope follower.
					mEnvelope = level > mEnvelope
						? mEnvelope + mAttackCoeff * (level - mEnvelope)
						: mEnvelope + mReleaseCoeff * (level - mEnvelope);

					// 2. Desired gain: keep the measured envelope below the ceiling
					//    (infinite-ratio limiting, driven by the envelope).
					//    A soft knee of width kneeDB lies entirely below the ceiling:
					//    at (ceiling - knee) the exponential curve leaves the unity
					//    line with matching level and slope (no corner) and approaches
					//    the ceiling asymptotically from below, so limiting begins
					//    before the ceiling is reached while the output never exceeds
					//    it for any envelope level. kneeDB = 0 keeps the hard knee.
					auto targetGain = 1.f;
					auto levelDB = toDB(mEnvelope);
					const auto kneeDB = mKnee.load();
					if (kneeDB > 0.f)
					{
						if (levelDB > ceilingDB - kneeDB)
						{
							auto gainDB = (ceilingDB - levelDB)
								- kneeDB * expf((ceilingDB - kneeDB - levelDB) / kneeDB);
							targetGain = powf(10.f, gainDB / 20.f);
						}
					}
					else if (mEnvelope > ceiling)
						targetGain = ceiling / mEnvelope;

					// 3. Smooth the gain (attack on reduction, release on recovery).
					mGain = targetGain < mGain
						? mGain + mAttackCoeff * (targetGain - mGain)
						: mGain + mReleaseCoeff * (targetGain - mGain);

					// 4. Track the average gain reduction (dB) for makeup gain.
					auto reductionDB = std::min(60.f, -toDB(mGain));
					mMakeupGain += mMakeupCoeff * (reductionDB - mMakeupGain);

					// 5. Apply the gain plus makeup, then a per-sample safety clamp.
					auto makeup = makeupEnabled ? powf(10.f, mMakeupGain / 20.f) : 1.f;
					auto out = in * mGain * makeup;
					if (std::fabs(out) > ceiling)
						out = std::copysign(ceiling, out);

					outputBuffer[i] = out;
					mGainReductionDB = -toDB(mGain);
				}
			}

			else
			{
				// ---- Compressor: standard dynamic compression ----
				const auto threshold = mThreshold.load();
				const auto ratio = mRatio.load();
				const auto makeupEnabled = mMakeupGainEnabled.load();

				for (auto i = 0; i < outputBuffer.size(); ++i)
				{
					auto in = (*inputBuffer)[i];

					// 1. Measure the input level with a fast/slow envelope follower.
					auto level = std::fabs(in);
					mEnvelope = level > mEnvelope
						? mEnvelope + mAttackCoeff * (level - mEnvelope)
						: mEnvelope + mReleaseCoeff * (level - mEnvelope);

					// 2. Compute the desired gain from the compression law (linear domain).
					auto levelDB = toDB(mEnvelope);
					auto targetGain = 1.f;
					if (levelDB > threshold)
					{
						auto excess = levelDB - threshold;
						auto reduced = excess / ratio;
						auto gainReductionDB = excess - reduced;
						// Convert the gain reduction from dB to linear: 10^(dB / 20).
						targetGain = powf(10.f, -gainReductionDB / 20.f);
					}

					// 3. Smooth the gain: attack on reduction, release on release.
					mGain = targetGain < mGain
						? mGain + mAttackCoeff * (targetGain - mGain)
						: mGain + mReleaseCoeff * (targetGain - mGain);

					// 4. Track the average gain reduction (dB) for makeup gain.
					auto reductionDB = std::min(60.f, -toDB(mGain));
					mMakeupGain += mMakeupCoeff * (reductionDB - mMakeupGain);

					// 5. Apply the gain plus makeup (makeup compensates the average reduction).
					auto makeup = makeupEnabled ? powf(10.f, mMakeupGain / 20.f) : 1.f;
					outputBuffer[i] = in * mGain * makeup;

					mGainReductionDB = -toDB(mGain);
				}
			}

			if (mDryWet.isRamping())
				for (int i = 0; i < outputBuffer.size(); ++i)
					outputBuffer[i] = math::lerp((*inputBuffer)[i], outputBuffer[i], mDryWet.getNextValue());
		}

	}

}
