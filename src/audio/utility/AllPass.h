#pragma once

#include <cassert>
#include <audio/utility/audiotypes.h>

namespace nap
{

	namespace audio
	{

		class AllPass
		{
		public:
			AllPass() = default;
			AllPass(int maxDelay) { reset(maxDelay); }

			void reset(int maxDelay)
			{
				mInputBuffer.resize(maxDelay, 0.f);
				mOutputBuffer.resize(maxDelay, 0.f);
				mBufferIndex = 0;
			}

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

			void setGain(ControllerValue value) { mGain = value; }
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