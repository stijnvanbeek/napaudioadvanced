/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <cassert>
#include <audio/utility/audiotypes.h>

namespace nap
{

	namespace audio
	{

        /**
         * Allpass filter object.
         * Call the process() method in order to process a single input sample.
         */
		class AllPass
		{
		public:
			AllPass() = default;

			/**
			 * Constructor
			 * @param maxDelay Size of the delay lines (and therefore maximum delay) in samples.
			 */
			AllPass(int maxDelay) { reset(maxDelay); }

			/**
			 * Reset the buffers to zero.
			 * @param maxDelay Size of the delay lines (and therefore maximum delay) in samples.
			 */
			void reset(int maxDelay)
			{
				mInputBuffer.resize(maxDelay, 0.f);
				mOutputBuffer.resize(maxDelay, 0.f);
				mBufferIndex = 0;
			}

			/**
			 * Process a single input sample and return the output sample of the filter
			 * @param input Value of the input sample
			 * @return Value of the output sample
			 */
			SampleValue process(SampleValue input)
			{
				int readIndex = mBufferIndex - mDelay;
				if (readIndex < 0)
					readIndex += mInputBuffer.size();
				SampleValue output = -mGain * input + mInputBuffer[readIndex] + mGain * mOutputBuffer[readIndex];
				mInputBuffer[mBufferIndex] = input;
				mOutputBuffer[mBufferIndex] = output;
				mBufferIndex++;
				if (mBufferIndex >= mInputBuffer.size())
					mBufferIndex = 0;
				return output;
			}

			/**
			 * Set the gain multiplier of the filter
			 * @param value New gain multiplier value
			 */
			void setGain(ControllerValue value) { mGain = value; }

			/**
			 * Sets the delay time in samples of the allpass filter
			 * @param value Delay time in samples
			 */
			void setDelay(int value) { assert(value <= mInputBuffer.size()); mDelay = value; }

		private:
			ControllerValue mGain = 1.f;
			int mDelay = 0;
			SampleBuffer mInputBuffer;
			SampleBuffer mOutputBuffer;
			int mBufferIndex = 0;
		};

	}

}