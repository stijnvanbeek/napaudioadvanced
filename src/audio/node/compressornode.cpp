/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "compressornode.h"


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::CompressorNode)
    RTTI_PROPERTY("audioInput", &nap::audio::CompressorNode::audioInput, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("audioOutput", &nap::audio::CompressorNode::audioOutput, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_FUNCTION("setRatio", &nap::audio::CompressorNode::setRatio)
	RTTI_FUNCTION("setThreshold", &nap::audio::CompressorNode::setThreshold)
	RTTI_FUNCTION("setAttack", &nap::audio::CompressorNode::setAttack)
	RTTI_FUNCTION("setRelease", &nap::audio::CompressorNode::setRelease)
RTTI_END_CLASS

namespace nap {
    namespace audio {
        
        FaustCompressor::FaustCompressor(int samplerate)
        {
            fSamplingFreq = samplerate;
            
            // faust constants
            fConst0 = fmin(192000.0f, fmax(1.0f, float(fSamplingFreq)));
            fConst1 = (2.0f / fConst0);
            fConst2 = (1.0f / fConst0);
        }

        
        void FaustCompressor::compute(int count, float* input0, float* output0)
        {
            float fSlow0 = float(fVslider0);
            float fSlow1 = expf((0.0f - (fConst1 / fSlow0)));
            float fSlow2 = (((1.0f / float(fVslider1)) + -1.0f) * (1.0f - fSlow1));
            float fSlow3 = expf((0.0f - (fConst2 / fSlow0)));
            float fSlow4 = expf((0.0f - (fConst2 / float(fVslider2))));
            float fSlow5 = float(fVslider3);
            for (int i = 0; (i < count); i = (i + 1)) {
                float fTemp0 = float(input0[i]);
                float fTemp1 = fabsf(fTemp0);
                float fTemp2 = ((fRec1[1] > fTemp1)?fSlow4:fSlow3);
                fRec2[0] = ((fRec2[1] * fTemp2) + ((1.0f - fTemp2) * fTemp1));
                fRec1[0] = fRec2[0];
                fRec0[0] = ((fSlow1 * fRec0[1]) + (fSlow2 * fmax(((20.0f * log10f(fRec1[0])) - fSlow5), 0.0f)));
                output0[i] = float((powf(10.0f, (0.0500000007f * fRec0[0])) * fTemp0));
                fRec2[1] = fRec2[0];
                fRec1[1] = fRec1[0];
                fRec0[1] = fRec0[0];
            }
        }


        void CompressorNode::process()
        {
            auto& inputBuffer = *audioInput.pull();
            auto& outputBuffer = getOutputBuffer(audioOutput);

            // Converts std::vector to float arrays
            float* inputArray = &inputBuffer[0];
            float* outputArray = &outputBuffer[0];

            faustCompressor.compute(getBufferSize(), inputArray, outputArray);
        }
    }
}
