#pragma once

#include <audio/utility/onepole.h>
#include <audio/utility/vectordelay.h>
#include <audio/utility/fastlinearsmoothedvalue.h>

#include <audio/utility/audiofunctions.h>

namespace nap
{

	namespace audio
	{

		template <typename real>
		class KarplusStrong
		{
		public:
			KarplusStrong() = default;

			void reset(int maxDelay)
			{
				int size = 2048;
				while (size < maxDelay)
					size *= 2;
				mDelay = std::make_unique<VectorDelay<real>>(size);
			}

			real processPositive(const real& input)
			{
				mTime.update();
				real value = mDelay->readInterpolating(mTime.getNextValue()) * mFeedback;
				value = mDampingFilter.process(value);
				mDelay->write(input + value);
				return value;
			}

			real processNegative(const real& input)
			{
				mTime.update();
				auto value = mDelay->readInterpolating(mTime.getNextValue()) * mFeedback;
				value = mDampingFilter.process(value);
				mDelay->write(input - value);
				return value;
			}

			void setDelayTime(real sampleTime, int stepCount)
			{
				assert(sampleTime <= mDelay->getMaxDelay());
				mTime.setStepCount(stepCount);
				mTime.setValue(sampleTime);
			}

			void setDamping(float cutoffFrequency, float sampleRate) { mDampingFilter.setCutoffFrequency(cutoffFrequency, sampleRate); }

			void setFeedback(real feedback)
			{
				assert(feedback < 1.f);
				mFeedback = feedback;
			}

			void flush() { mDelay->clear(); }

		private:
			OnePoleLowPass<real> mDampingFilter;
			std::unique_ptr<VectorDelay<real>> mDelay = nullptr;
			real mFeedback = { 0.f };
			FastLinearSmoothedValue<real> mTime = { 0.f, 44 };
		};

	}

}
