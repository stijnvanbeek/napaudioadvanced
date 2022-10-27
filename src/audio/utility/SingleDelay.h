#pragma once

#include <audio/utility/audiotypes.h>
#include <audio/utility/delay.h>
#include <memory>

namespace nap
{

	namespace audio
	{

		class SingleDelay
		{
		public:
			SingleDelay() = default;

			void reset(int maxDelay)
			{
				int size = 2048;
				while (size < maxDelay)
					size *= 2;
				mDelay = std::make_unique<Delay>(size);
			}

			void setDelay(ControllerValue sampleTime)
			{
				assert(mTime <= mDelay->getMaxDelay());
				mTime = sampleTime;
			}

			SampleValue process(SampleValue input)
			{
				mDelay->write(input);
				return mDelay->read(mTime);
			}

			SampleValue processInterpolating(SampleValue input)
			{
				mDelay->write(input);
				return mDelay->readInterpolating(mTime);
			}

		private:
			std::unique_ptr<Delay> mDelay = nullptr;
			ControllerValue mTime = 0.f;
		};

	}

}