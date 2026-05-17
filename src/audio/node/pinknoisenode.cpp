#include "pinknoisenode.h"

#include <audio/utility/audiofunctions.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::PinkNoiseNode)
RTTI_END_CLASS

namespace nap
{

    namespace audio
    {

        void PinkNoiseNode::process()
        {
            auto& buffer = getOutputBuffer(output);
            for (auto i = 0; i < buffer.size(); ++i)
            {
                // Start with white noise
                float whiteNoise = math::random(-1.f, 1.f);
                float pinkNoise = 0.f;

                // Process it through the cascade of onepole lowpass filters
                auto amplitude = mAmplitudes.begin();
                for (auto& filter : mFilters)
                    pinkNoise += filter->process(whiteNoise) * *amplitude++;

                buffer[i] = pinkNoise;
            }
        }


        void PinkNoiseNode::sampleRateChanged(float sampleRate)
        {
            // Initialize a cascade of onepole lowpass filters for every 2 octaves down from nyquist
            mFilters.clear();
            auto f = sampleRate / 2.f;
            while (f > 20.f)
            {
                auto filter = std::make_unique<OnePoleLowPass<float>>();
                filter->setCutoffFrequency(f, sampleRate);
                mFilters.emplace_back(std::move(filter));
                mAmplitudes.emplace_back(0.f);
                f /= 4.f;
            }

            // Initialize the amplitudes of the filters, -6dB every 2 octaves downwards
            float dB = 0.f;
            for (auto amplitude = mAmplitudes.rbegin(); amplitude != mAmplitudes.rend(); ++amplitude)
            {
                *amplitude = dbToA(dB);
                dB -= 6.f;
            }
        }

    }

}