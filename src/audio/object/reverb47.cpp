/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "reverb47.h"

RTTI_BEGIN_CLASS(nap::audio::verb47::Reverb47)
        RTTI_PROPERTY("CorrelationMultiplier", &nap::audio::verb47::Reverb47::mCorrelationMultiplier, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("DiffusionCrossover", &nap::audio::verb47::Reverb47::mDiffusionCrossover, nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Size", &nap::audio::verb47::Reverb47::mSize, nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Decay", &nap::audio::verb47::Reverb47::mDecay, nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Damping", &nap::audio::verb47::Reverb47::mDamping, nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Diffusion", &nap::audio::verb47::Reverb47::mDiffusion, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_DEFINE_CLASS(nap::audio::verb47::ReverbInstance47)

namespace nap
{
    namespace audio
    {

        namespace verb47
        {

            bool Reverb47::initNode(int channel, ReverbNode& node, utility::ErrorState& errorState)
            {
                ReverbSettings settings;
                settings.multiply(mCorrelationMultiplier[channel % mCorrelationMultiplier.size()]);
                node.applySettings(settings);
                node.setSize(mSize);
                node.setDecay(mDecay);
                node.setDamping(mDamping);
                node.setDiffusion(mDiffusion);
                node.setModulationAmplitude(0.1f);
                node.setModulationSpeed(0.5f);
                if (mInput != nullptr)
                {
                    node.audioInput.connect(*mInput->getInstance()->getOutputForChannel(channel % mInput->getInstance()->getChannelCount()));
                }

                if (mDiffusionCrossover)
                {
                    if (!mInitializedNodes.empty())
                    {
                        auto previousNode = mInitializedNodes.back();
                        node.diffusionInput1.connect(previousNode->diffusionOutput1);
                        node.diffusionInput2.connect(previousNode->diffusionOutput2);
                        node.diffusionInput3.connect(previousNode->diffusionOutput3);
                    }

                    mInitializedNodes.emplace_back(&node);

                    if (mInitializedNodes.size() == mChannelCount)
                    {
                        auto firstNode = *mInitializedNodes.begin();
                        firstNode->diffusionInput1.connect(node.diffusionOutput1);
                        firstNode->diffusionInput2.connect(node.diffusionOutput2);
                        firstNode->diffusionInput3.connect(node.diffusionOutput3);
                        mInitializedNodes.clear();
                    }
                }

                return true;
            }


        }

    }

}
