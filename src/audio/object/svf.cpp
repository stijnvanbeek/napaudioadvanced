/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "svf.h"

RTTI_BEGIN_CLASS(nap::audio::SVF)
    RTTI_PROPERTY("Mode", &nap::audio::SVF::mMode, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Frequency", &nap::audio::SVF::mFrequency, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Resonance", &nap::audio::SVF::mResonance, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Gain", &nap::audio::SVF::mGain, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::ParallelNodeObjectInstance<nap::audio::SVFNode>)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        bool SVF::initNode(int channel, SVFNode& node, utility::ErrorState& errorState)
        {
            node.setMode(mMode);
            node.setFrequency(mFrequency[channel % mFrequency.size()]);
            node.setResonance(mResonance[channel % mResonance.size()]);
            node.setGain(mGain[channel % mGain.size()]);
            if (mInput != nullptr)
                node.audioInput.connect(*mInput->getInstance()->getOutputForChannel(channel % mInput->getInstance()->getChannelCount()));
            return true;
        }
    }
    
}

