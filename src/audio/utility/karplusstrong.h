/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <audio/utility/onepole.h>
#include <audio/utility/vectordelay.h>
#include <audio/utility/fastlinearsmoothedvalue.h>

#include <audio/utility/audiofunctions.h>

namespace nap
{

	namespace audio
	{

	    /**
	     * Karplus strong filter algorithm
         * @tparam real Should be float, @float4 or @float8.
	     */
		template <typename real>
		class KarplusStrong
		{
		public:
			KarplusStrong() = default;

			/**
			 * Reset the filter
             * Should only be called from the audio thread.
			 * @param maxDelay New size of the delay line, therefore the maximum delay value in samples
			 */
			void reset(int maxDelay)
			{
				int size = 2048;
				while (size < maxDelay)
					size *= 2;
				mDelay = std::make_unique<VectorDelay<real>>(size);
			}

			/**
			 * Processes a single sample input value.
			 * The addition "positive" means that the feedback signal is added to the input signal, therefore creating both even and odd harmonics of the frequency corresponding to the period of the delay time.
			 * @param input Input sample value to be processed by the filter
			 * @return Output value of the filter
			 */
			real processPositive(const real& input)
			{
				mTime.update();
				real value = mDelay->readInterpolating(mTime.getNextValue()) * mFeedback;
				value = mDampingFilter.process(value);
				mDelay->write(input + value);
				return value;
			}

            /**
             * Processes a single sample input value.
             * The addition "negative" means that the feedback signal is subtracted to the input signal, therefore creating odd harmonics of the frequency corresponding to the period of the delay time.
             * @param input Input sample value to be processed by the filter
             * @return Output value of the filter
             */
			real processNegative(const real& input)
			{
				mTime.update();
				auto value = mDelay->readInterpolating(mTime.getNextValue()) * mFeedback;
				value = mDampingFilter.process(value);
				mDelay->write(input - value);
				return value;
			}

			/**
			 * Set the new delay time with linear interpolation
			 * @param sampleTime New delay time in samples
			 * @param stepCount Number of steps in samples to reach the new delay time
			 */
			void setDelayTime(real sampleTime, int stepCount)
			{
				assert(sampleTime <= mDelay->getMaxDelay());
				mTime.setStepCount(stepCount);
				mTime.setValue(sampleTime);
			}

			/**
			 * Set the damping or cutoff frequency of the lowpass filter within the feedback loop.
			 * @param cutoffFrequency New value in Hz
			 * @param sampleRate Current samplerate the filter is running on
			 */
			void setDamping(float cutoffFrequency, float sampleRate) { mDampingFilter.setCutoffFrequency(cutoffFrequency, sampleRate); }

			/**
			 * Set the feedback multiplier value.
			 * @param feedback New feedback multiplier
			 */
			void setFeedback(real feedback)
			{
				assert(feedback < 1.f);
				mFeedback = feedback;
			}

			/**
			 * Sets the values in the delay line to zero.
			 */
			void flush() { mDelay->clear(); }

		private:
			OnePoleLowPass<real> mDampingFilter;
			std::unique_ptr<VectorDelay<real>> mDelay = nullptr;
			std::atomic<real> mFeedback = { 0.f };
			FastLinearSmoothedValue<real> mTime = { 0.f, 44 };
		};

	}

}
