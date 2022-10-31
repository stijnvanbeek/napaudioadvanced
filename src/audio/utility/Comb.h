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
			void reset(int maxDelay)
			{
				mBuffer.resize(maxDelay, 0.f);
				mBufferIndex = 0;
			}

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

			void setDelay(int delay) { assert(delay < mBuffer.size()); mDelay = delay; }
			void setGain(ControllerValue value) { mGain = value; }
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