/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "svfnode.h"

#include <cmath>

#include <audio/core/audionodemanager.h>
#include <mathutils.h>

RTTI_BEGIN_ENUM(nap::audio::SVFNode::EMode)
	RTTI_ENUM_VALUE(nap::audio::SVFNode::EMode::LowPass, "LowPass"),
	RTTI_ENUM_VALUE(nap::audio::SVFNode::EMode::HighPass, "HighPass"),
	RTTI_ENUM_VALUE(nap::audio::SVFNode::EMode::BandPass, "BandPass"),
	RTTI_ENUM_VALUE(nap::audio::SVFNode::EMode::Notch, "Notch"),
	RTTI_ENUM_VALUE(nap::audio::SVFNode::EMode::Peak, "Peak")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::SVFNode)
	RTTI_PROPERTY("audioInput", &nap::audio::SVFNode::audioInput, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("audioOutput", &nap::audio::SVFNode::audioOutput, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_FUNCTION("setMode", &nap::audio::SVFNode::setMode)
	RTTI_FUNCTION("setFrequency", &nap::audio::SVFNode::setFrequency)
	RTTI_FUNCTION("setResonance", &nap::audio::SVFNode::setResonance)
	RTTI_FUNCTION("setGain", &nap::audio::SVFNode::setGain)
	RTTI_FUNCTION("setSmoothing", &nap::audio::SVFNode::setSmoothing)
	RTTI_FUNCTION("prepare", &nap::audio::SVFNode::prepare)
	RTTI_FUNCTION("getMode", &nap::audio::SVFNode::getMode)
	RTTI_FUNCTION("getFrequency", &nap::audio::SVFNode::getFrequency)
	RTTI_FUNCTION("getResonance", &nap::audio::SVFNode::getResonance)
	RTTI_FUNCTION("getGain", &nap::audio::SVFNode::getGain)
RTTI_END_CLASS

namespace nap
{
	namespace audio
	{

		SVFNode::SVFNode(NodeManager& nodeManager) : Node(nodeManager)
		{
			calcCoeffs(mFrequency, mResonance);
			mLastFrequency = mFrequency;
			mLastResonance = mResonance;
			mRecalcCoeffs = false;
			setSmoothing(mSmoothing);
			mFrequencyValue.reset(mFrequency);
			mResonanceValue.reset(mResonance);
			mGainValue.reset(mGain);
		}


		void SVFNode::process()
		{
			if (mIsDirty.check())
			{
				mFrequencyValue.setValue(mFrequency);
				mResonanceValue.setValue(mResonance);
				mGainValue.setValue(mGain);
			}

			auto inputBuffer = audioInput.pull();
			auto& outputBuffer = getOutputBuffer(audioOutput);

			EMode mode = mMode;
			for (auto i = 0; i < outputBuffer.size(); ++i)
			{
				// Ramp the parameters per sample and recalculate the coefficients from the
				// ramping values, so every intermediate state is a stable, valid filter.
				ControllerValue frequency = mFrequencyValue.getNextValue();
				ControllerValue resonance = mResonanceValue.getNextValue();
				if (mRecalcCoeffs || frequency != mLastFrequency || resonance != mLastResonance)
				{
					calcCoeffs(frequency, resonance);
					mLastFrequency = frequency;
					mLastResonance = resonance;
					mRecalcCoeffs = false;
				}

				ControllerValue input = inputBuffer != nullptr ? (*inputBuffer)[i] : 0;

				// Trapezoidal state variable filter, after Andrew Simper's TPT formulation.
				// mIc1 integrates the bandpass, mIc2 the lowpass; both half-updated states
				// are preserved across parameter changes, so fast modulation stays click-free.
				ControllerValue v3 = input - mIc2;
				ControllerValue v1 = a1 * mIc1 + a2 * v3;
				ControllerValue v2 = a2 * mIc1 + a3 * v3;
				ControllerValue lowPass = mIc2 + v2;
				mIc1 = 2 * v1 - mIc1;
				mIc2 = mIc2 + 2 * v2;

				ControllerValue output;
				switch (mode) {
					case EMode::LowPass:
						output = lowPass;
						break;
					case EMode::HighPass:
						output = v3 - k * v1 - v2;
						break;
					case EMode::BandPass:
						output = v1;
						break;
					case EMode::Notch:
						output = input - k * v1;
						break;
					case EMode::Peak:
						output = input - k * v1 + lowPass;
						break;
					default:
						output = lowPass;
						break;
				}

				outputBuffer[i] = output * mGainValue.getNextValue();
			}
		}


		void SVFNode::sampleRateChanged(float sampleRate)
		{
			setSmoothing(mSmoothing);
			mRecalcCoeffs = true;
		}


		void SVFNode::prepare(EMode mode, ControllerValue frequency, ControllerValue resonance, ControllerValue gain)
		{
			mMode = mode;
			mFrequency = frequency;
			mResonance = resonance;
			mGain = gain;
			mIc1 = 0;
			mIc2 = 0;
			calcCoeffs(mFrequency, mResonance);
			mLastFrequency = mFrequency;
			mLastResonance = mResonance;
			mRecalcCoeffs = false;
			mFrequencyValue.reset(mFrequency);
			mResonanceValue.reset(mResonance);
			mGainValue.reset(mGain);
		}


		void SVFNode::setMode(EMode mode)
		{
			mMode = mode;
			mIsDirty.set();
		}


		void SVFNode::setFrequency(ControllerValue frequency)
		{
			mFrequency = math::max<ControllerValue>(frequency, 1.f);
			mIsDirty.set();
		}


		void SVFNode::setResonance(ControllerValue resonance)
		{
			mResonance = resonance;
			mIsDirty.set();
		}


		void SVFNode::setGain(ControllerValue gain)
		{
			mGain = gain;
			mIsDirty.set();
		}


		void SVFNode::set(EMode mode, ControllerValue frequency, ControllerValue resonance, ControllerValue gain)
		{
			mMode = mode;
			mFrequency = math::max<ControllerValue>(frequency, 1.f);
			mResonance = resonance;
			mGain = gain;
			mIsDirty.set();
		}


		void SVFNode::setSmoothing(TimeValue smoothing)
		{
			mSmoothing = smoothing;
			int stepCount = smoothing * getNodeManager().getSamplesPerMillisecond();
			mFrequencyValue.setStepCount(stepCount);
			mResonanceValue.setStepCount(stepCount);
			mGainValue.setStepCount(stepCount);
		}


		void SVFNode::calcCoeffs(ControllerValue frequency, ControllerValue resonance)
		{
			// Keep the cutoff below Nyquist, so the filter remains stable for any parameter
			// value the smoother can pass through.
			frequency = math::clamp<ControllerValue>(frequency, 1.f, 0.495f * getSampleRate());

			// Damping mapped from the resonance parameter exactly as FilterNode maps it:
			// resonance 0 is the Butterworth response (k = sqrt(2)), 30 approaches
			// self-oscillation (k near zero). k stays positive for any clamped value.
			resonance = math::clamp<ControllerValue>(resonance, 0.f, 30.f);
			k = math::SQRT2 * pow(10.f, -resonance * 0.1f);

			// Cutoff term of the trapezoidal integrators, frequency warped to the digital domain.
			g = tan(math::PI * frequency / getSampleRate());

			a1 = 1 / (1 + g * (g + k));
			a2 = g * a1;
			a3 = g * a2;
		}


	}
}
