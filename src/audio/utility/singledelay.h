/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <audio/utility/audiotypes.h>
#include <audio/utility/delay.h>
#include <memory>

namespace nap
{

	namespace audio
	{

	    /**
	     * Single delay algorithm without feedback
	     */
		class SingleDelay
		{
		public:
			SingleDelay() = default;

			/**
			 * Flushes the delay line
             * Should only be called from the audio thread.
			 * @param maxDelay Size of the delay line, also maximum delay time in samples.
			 */
			void reset(int maxDelay)
			{
				int size = 2048;
				while (size < maxDelay)
					size *= 2;
				mDelay = std::make_unique<Delay>(size);
			}

			/**
			 * Set the delay time
			 * @param sampleTime New delay time in samples
			 */
			void setDelay(ControllerValue sampleTime)
			{
				assert(mTime <= mDelay->getMaxDelay());
				mTime = sampleTime;
			}

			/**
			 * Processes a single input sample
			 * @param input Input sample value
			 * @return Output value
			 */
			SampleValue process(SampleValue input)
			{
				mDelay->write(input);
				return mDelay->read(mTime);
			}

			/**
			 * Process a single input sample using interpolation in case the delay is modulating
			 * @param input Input sample value
			 * @return Output value
			 */
			SampleValue processInterpolating(SampleValue input)
			{
				mDelay->write(input);
				return mDelay->readInterpolating(mTime);
			}

		private:
			std::unique_ptr<Delay> mDelay = nullptr;
			std::atomic<ControllerValue> mTime = 0.f;
		};

	}

}
