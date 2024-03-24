/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <audio/utility/audiotypes.h>

namespace nap
{

	namespace audio
	{

		/**
		 * Feedforward comb filter
		 */
		class Comb
		{
		public:
		    /**
		     * Reset the delay lines by flushing them with zero's
		     * @param maxDelay Size of the delay lines in samples, therefore the maximum delay time in samples
		     */
			void reset(int maxDelay)
			{
				mBuffer.resize(maxDelay, 0.f);
				mBufferIndex = 0;
			}

			/**
			 * Process a single sample input
			 * @param input Value of the input sample
			 * @return Output value of the comb filter
			 */
			SampleValue process(SampleValue input)
			{
				int readIndex = mBufferIndex - mDelay;
				if (readIndex < 0)
					readIndex += mBuffer.size();

				mBuffer[mBufferIndex] = input;
				SampleValue result = mGain * input + mFeedforward * mBuffer[readIndex];

				mBufferIndex++;
				if (mBufferIndex >= mBuffer.size())
					mBufferIndex = 0;

				return result;
			}

			/**
			 * Set delay time
			 * @param delay Discrete delay time in samples
			 */
			void setDelay(int delay) { assert(delay < mBuffer.size()); mDelay = delay; }

			/**
			 * Sets the gain multiplier of the filter
			 * @param value Gain multiplier value
			 */
			void setGain(ControllerValue value) { mGain = value; }

			/**
			 * Sets the feedforward gain multiplier
			 * @param value Gain multiplier of the feed forward signal
			 */
			void setFeedforward(ControllerValue value) { mFeedforward = value; }

		private:
			SampleBuffer mBuffer;
			int mBufferIndex = 0;
			int mDelay = 0;
			ControllerValue mGain = 1.f;
			ControllerValue mFeedforward = 1.f;
		};

	}

}