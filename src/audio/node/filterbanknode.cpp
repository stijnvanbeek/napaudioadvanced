/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "filterbanknode.h"

#include <cmath>
#include <audio/core/audionodemanager.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::FilterBankNode)
    RTTI_PROPERTY("input", &nap::audio::FilterBankNode::audioInput, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("output", &nap::audio::FilterBankNode::output, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_FUNCTION("setFilterCount", &nap::audio::FilterBankNode::setFilterCount)
    RTTI_FUNCTION("setParameters", &nap::audio::FilterBankNode::setParameters)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        float8 makeFloat8(const std::vector<float>& list)
        {
            float8 result;
            for (auto i = 0; i < 8; ++i)
                result[i] = list[i % list.size()];
            return result;
        }
        
        
        FilterBank::~FilterBank()
        {
			UpdateFunction* functionToDelete = nullptr;
            while (mDeletionQueue.try_dequeue(functionToDelete))
                delete functionToDelete;
		}


		void FilterBank::setFilterCount(unsigned int count)
		{
			if (count <= 8)
				mFilterCount = count;
			else
				mFilterCount = 8;
		}


		void FilterBank::setParameters(const std::vector<ControllerValue>& aCenterFrequency, const std::vector<ControllerValue>& aBandWidth, const std::vector<ControllerValue>& aGain, float aSampleRate)
        {
            float8 centerFrequency = makeFloat8(aCenterFrequency);
            float8 bandWidth = makeFloat8(aBandWidth);
            float8 gain = makeFloat8(aGain);
            float8 scaledGain = gain * powVec(float8(10000.0) / bandWidth, float8(0.5));
            
            float8 sampleRate = float8(aSampleRate);
            float8 zero = float8(0);
            float8 one = float8(1);
            float8 two = float8(2);

            float8 c = one / tanVec(float8(math::PI) * bandWidth / sampleRate);
            float8 d = two * cosVec(float8(math::PIX2) * centerFrequency / sampleRate);
            float8 a0 = one / (one + c);
            float8 a1 = zero;
            float8 a2 = zero - a0;
            float8 b1 = a2 * c * d;
            float8 b2 = a0 * (c - one);
            auto biquadPtr = &mFilter;
            
            auto updateFunction = new std::function<void()>(
                [a0, a1, a2, b1, b2, scaledGain, biquadPtr](){
                  biquadPtr->setCoefficients(a0, a1, a2, b1, b2, scaledGain);
                }
            );
			
            updateFunction = mUpdateFunction.exchange(updateFunction);
			
            delete updateFunction;
            updateFunction = nullptr;

            UpdateFunction * functionToDelete = nullptr;
            while (mDeletionQueue.try_dequeue(functionToDelete))
                delete functionToDelete;
        }


		void FilterBank::setLowShelf(ControllerValue cutoffFrequency, ControllerValue gain, float sampleRate)
		{
			mLowShelfGain = gain;
			mLowShelf.setCutoffFrequency(cutoffFrequency, sampleRate);
		}


		void FilterBank::processBuffer(SampleBuffer& inputBuffer, SampleBuffer& outputBuffer)
		{
			auto filterCount = mFilterCount.load();

			UpdateFunction * updateFunction = mUpdateFunction.exchange(nullptr);
			if (updateFunction)
			{
				(*updateFunction)();
				mDeletionQueue.enqueue(updateFunction);
			}

			for (auto i = 0; i < outputBuffer.size(); ++i)
			{
				const float8 inputValue(inputBuffer[i]);
				const float8 outputValue = mFilter.process(inputValue);
				float result = 0;
				for (auto filterIndex = 0; filterIndex < filterCount; ++filterIndex)
					result += outputValue[filterIndex];
				result += mLowShelf.process(inputBuffer[i]) * mLowShelfGain;
				outputBuffer[i] = result;
			}
		}
        
        
        void FilterBankNode::process()
        {
            auto& inputBuffer = *audioInput.pull();
            auto& outputBuffer = getOutputBuffer(output);
			mFilterBank.processBuffer(inputBuffer, outputBuffer);
        }
        
        
    }
    
}
