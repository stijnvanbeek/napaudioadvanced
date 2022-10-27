#include "reverbnode47.h"

#include <audio/utility/audiofunctions.h>
#include <audio/core/audionodemanager.h>

RTTI_BEGIN_STRUCT(nap::audio::verb47::ReverbSettings)
        RTTI_PROPERTY("InputAllPassDelays", &nap::audio::verb47::ReverbSettings::mInputAllPassDelays, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("InputAllPassGains", &nap::audio::verb47::ReverbSettings::mInputAllPassGains, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("SizeAllPassDelays", &nap::audio::verb47::ReverbSettings::mSizeAllPassDelays, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("SizeAllPassGains", &nap::audio::verb47::ReverbSettings::mSizeAllPassGains, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("DelaySizeMultipliers", &nap::audio::verb47::ReverbSettings::mDelaySizeMultipliers, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("DiffusorDelayMultipliers", &nap::audio::verb47::ReverbSettings::mDiffusorDelayMultipliers, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("DiffusorGain", &nap::audio::verb47::ReverbSettings::mGain, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::verb47::ReverbNode)
        RTTI_FUNCTION("setSize", &nap::audio::verb47::ReverbNode::setSize)
        RTTI_FUNCTION("setDecay", &nap::audio::verb47::ReverbNode::setDecay)
        RTTI_FUNCTION("setDamping", &nap::audio::verb47::ReverbNode::setDamping)
        RTTI_FUNCTION("setDiffusion", &nap::audio::verb47::ReverbNode::setDiffusion)
        RTTI_FUNCTION("setModulationAmplitude", &nap::audio::verb47::ReverbNode::setModulationAmplitude)
        RTTI_FUNCTION("setModulationSpeed", &nap::audio::verb47::ReverbNode::setModulationSpeed)
        RTTI_FUNCTION("setLowCut", &nap::audio::verb47::ReverbNode::setLowCut)
RTTI_END_CLASS

namespace nap
{

    namespace audio
    {

        namespace verb47
        {

            ReverbNode::ReverbNode(NodeManager& nodeManager) : Node(nodeManager)
            {
                sampleRateChanged(nodeManager.getSampleRate());
            }


            void ReverbNode::setSize(ControllerValue value)
            {
                mSize = math::fit(value, 0.f, 1.f, 0.01f, 1.6f);
                mSizeAllPasses[0].setDelay(mSize * mSettings.mSizeAllPassDelays[0] * mSamplesPerMillisecond);
                mSizeAllPasses[1].setDelay(mSize * mSettings.mSizeAllPassDelays[1] * mSamplesPerMillisecond);
                mDelays[1].setDelay(mSize * mSettings.mDelaySizeMultipliers[1] * mSamplesPerMillisecond);
                for (auto j = 0; j < mDiffusors.size(); ++j)
                    mDiffusors[j].setDelay(mDiffusion * mSettings.mDiffusorDelayMultipliers[j] * mSize * mSamplesPerMillisecond);
            }


            void ReverbNode::setDecay(ControllerValue value)
            {
                mDecay = math::fit<float>(value, 0.f, 1.f, 0.05f, 0.99f);
            }


            void ReverbNode::setDamping(ControllerValue value)
            {
                mDamping = math::fit(math::power(value, 0.5f), 0.f, 1.f, 22000.f, 20.f);
                mDampingOnePole.setCutoffFrequency(mDamping, getNodeManager().getSampleRate());
                mInputHighCutOnePole.setCutoffFrequency(mDamping, getNodeManager().getSampleRate());
            }


            void ReverbNode::setDiffusion(ControllerValue value)
            {
                mDiffusion = value;
                for (auto j = 0; j < mDiffusors.size(); ++j)
                    mDiffusors[j].setDelay(mDiffusion * mSettings.mDiffusorDelayMultipliers[j] * mSize * mSamplesPerMillisecond);
            }


            void ReverbNode::setModulationAmplitude(ControllerValue value)
            {
                mModulationBandWidth = math::fit(math::power(value, 2.f), 0.f, 1.f, 0.f, 1.f);
            }


            void ReverbNode::setModulationSpeed(ControllerValue value)
            {
                mModulationTime = math::fit(math::power(value, 0.1f), 0.f, 1.f, 2000.f, 1.f);
                mModulator.setStepCount(mModulationTime * mSamplesPerMillisecond);
                mModulationOnePole.setCutoffFrequency(100.f / mModulationTime, getNodeManager().getSampleRate());
            }


            void ReverbNode::setLowCut(ControllerValue value)
            {
                mInputLowCut = math::fit(math::power(value, 2.f), 0.f, 1.f, 20.f, 22000.f);
                mInputLowCutOnePole.setCutoffFrequency(mInputLowCut, getNodeManager().getSampleRate());
            }


            void ReverbNode::sampleRateChanged(float sampleRate)
            {
                mSamplesPerMillisecond = getNodeManager().getSamplesPerMillisecond();

                // In order to init all delay lines we choose a set of settings with multiplier 2, so we allocate the maximum size for a maximum multiplier of 2
                ReverbSettings maxSettings;
                maxSettings.multiply(2.f);

                for (auto i = 0; i < mInputAllPasses.size(); ++i)
                {
                    auto delay = maxSettings.mInputAllPassDelays[i] * mSamplesPerMillisecond;
                    mInputAllPasses[i].reset(delay);
                }

                for (auto i = 0; i < mSizeAllPasses.size(); ++i)
                    mSizeAllPasses[i].reset(200 * mSamplesPerMillisecond);

                mDelays[0].reset(2000.f * mSamplesPerMillisecond);
                mDelays[1].reset(1000.f * mSamplesPerMillisecond);

                for (auto i = 0; i < mDiffusors.size(); ++i)
                    mDiffusors[i].reset(maxSettings.mDiffusorDelayMultipliers[i] * mSamplesPerMillisecond);

                mFeedbackInput = 0.f;

                mModulator.setStepCount(mModulationTime * mSamplesPerMillisecond);
                mModulationOnePole.setCutoffFrequency(100.f / mModulationTime, getNodeManager().getSampleRate());

                mDampingOnePole.setCutoffFrequency(mDamping, getNodeManager().getSampleRate());

                mInputLowCutOnePole.setCutoffFrequency(mInputLowCut, getNodeManager().getSampleRate());
                mInputHighCutOnePole.setCutoffFrequency(mDamping, getNodeManager().getSampleRate());

                applySettingsToDSP();
            }


            void ReverbNode::applySettingsToDSP()
            {
                for (auto i = 0; i < mInputAllPasses.size(); ++i)
                {
                    mInputAllPasses[i].setGain(mSettings.mInputAllPassGains[i]);
                    auto delay = mSettings.mInputAllPassDelays[i] * mSamplesPerMillisecond;
                    mInputAllPasses[i].setDelay(delay);
                }

                for (auto i = 0; i < mSizeAllPasses.size(); ++i)
                    mSizeAllPasses[i].setGain(mSettings.mSizeAllPassGains[i]);

                mSizeAllPasses[0].setDelay(mSize * mSettings.mSizeAllPassDelays[0] * mSamplesPerMillisecond);
                mSizeAllPasses[1].setDelay(mSize * mSettings.mSizeAllPassDelays[1] * mSamplesPerMillisecond);
                mDelays[1].setDelay(mSize * mSettings.mDelaySizeMultipliers[1] * mSamplesPerMillisecond);
                for (auto j = 0; j < mDiffusors.size(); ++j)
                    mDiffusors[j].setDelay(mDiffusion * mSettings.mDiffusorDelayMultipliers[j] * mSize * mSamplesPerMillisecond);
            }


            void ReverbNode::process()
            {
                auto inputBuffer = audioInput.pull();
                if (inputBuffer == nullptr)
                    return;

                auto& outputBuffer = getOutputBuffer(audioOutput);
                auto& diffusionOutputBuffer1 = getOutputBuffer(diffusionOutput1);
                auto& diffusionOutputBuffer2 = getOutputBuffer(diffusionOutput2);
                auto& diffusionOutputBuffer3 = getOutputBuffer(diffusionOutput3);
                auto diffusionInputBuffer1 = diffusionInput1.pull();
                auto diffusionInputBuffer2 = diffusionInput2.pull();
                auto diffusionInputBuffer3 = diffusionInput3.pull();

                for (auto i = 0; i < outputBuffer.size(); ++i)
                {
                    // Perform the delay modulation
                    if (!mModulator.isRamping())
                        mModulator.setValue(math::random<float>(0.f, mModulationBandWidth));
                    auto modulation = mModulationOnePole.process(mModulator.getNextValue());
                    mDelays[0].setDelay(mSize * mSettings.mDelaySizeMultipliers[0] * (1 + modulation) * mSamplesPerMillisecond);

                    // Input filtering
                    auto value = (*inputBuffer)[i];
                    value = mInputLowCutOnePole.process(value);
                    value = mInputHighCutOnePole.process(value);

                    // Input allpass chain
                    for (auto& allpass : mInputAllPasses)
                        value = allpass.process(value);

                    // Allpass tuned to size
                    value = mSizeAllPasses[0].process(value + mFeedbackInput);
                    auto diffusionInput1 = value;
                    auto diffusionInput2 = value;

                    // Modulated delay
                    value = mDelays[0].processInterpolating(value);
                    diffusionOutputBuffer1[i] = value;

                    // Apply Damping
                    value = mDampingOnePole.process(value);

                    // Apply decay
                    value *= mDecay;

                    // Allpass tuned to size
                    value = mSizeAllPasses[1].process(value);
                    auto diffusionInput3 = value;
                    diffusionOutputBuffer2[i] = value;

                    // Delay tuned to size
                    value = mDelays[1].process(value);
                    mFeedbackInput = value;
                    auto diffusionInput4 = value;
                    diffusionOutputBuffer3[i] = value;

                    // Diffusion
                    auto diffusion5 = diffusionInputBuffer1 ? (*diffusionInputBuffer1)[i] : diffusionOutputBuffer1[i];
                    auto diffusion6 = diffusionInputBuffer2 ? (*diffusionInputBuffer2)[i] : diffusionOutputBuffer2[i];
                    auto diffusion7 = diffusionInputBuffer3 ? (*diffusionInputBuffer3)[i] : diffusionOutputBuffer3[i];
                    value = mDiffusors[0].process(diffusionInput1) + mDiffusors[1].process(diffusionInput2) + mDiffusors[3].process(diffusionInput4) - (mDiffusors[2].process(diffusionInput3) + mDiffusors[4].process(diffusion5) + mDiffusors[5].process(diffusion6) + mDiffusors[6].process(diffusion7));

                    // Output gain
                    value *= mSettings.mGain;

                    outputBuffer[i] = value;
                }
            }


            void ReverbSettings::multiply(float factor)
            {
                for (auto& value : mInputAllPassDelays)
                    value *= factor;
                for (auto& value : mSizeAllPassDelays)
                    value *= factor;
                for (auto& value : mDelaySizeMultipliers)
                    value *= factor;
                for (auto& value : mDiffusorDelayMultipliers)
                    value *= factor;
            }

        }

    }

}