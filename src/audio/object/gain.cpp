#include "gain.h"

RTTI_BEGIN_CLASS(nap::audio::Gain)
    RTTI_PROPERTY("Gain", &nap::audio::Gain::mGain, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Input", &nap::audio::Gain::mInput, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::ParallelNodeObjectInstance<nap::audio::GainNode>)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        bool Gain::initNode(int channel, GainNode& node, utility::ErrorState& errorState)
        {
            if (mInput != nullptr)
                node.audioInput.connect(*mInput->getInstance()->getOutputForChannel(channel % mInput->getInstance()->getChannelCount()));
            node.setGain(mGain[channel % mGain.size()], 0);

            return true;
        }
    }
    
}
