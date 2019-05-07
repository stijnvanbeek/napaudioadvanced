#include "mixer.h"

RTTI_BEGIN_CLASS(nap::audio::Mixer)
    RTTI_PROPERTY("ChannelCount", &nap::audio::Mixer::mChannelCount, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Inputs", &nap::audio::Mixer::mInputs, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
    }
    
}
