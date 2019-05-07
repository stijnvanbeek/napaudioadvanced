#include "audioobject.h"


// Nap includes
#include <entity.h>
#include <nap/core.h>
#include <nap/logger.h>

// Audio includes
#include <audio/service/audioservice.h>


// RTTI
RTTI_DEFINE_BASE(nap::audio::AudioObject)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AudioObjectInstance)
    RTTI_FUNCTION("getChannelCount", &nap::audio::AudioObjectInstance::getChannelCount)
    RTTI_FUNCTION("getInputChannelCount", &nap::audio::AudioObjectInstance::getInputChannelCount)
    RTTI_FUNCTION("getOutputForChannel", &nap::audio::AudioObjectInstance::tryGetOutputForChannel)
    RTTI_FUNCTION("getInputForChannel", &nap::audio::AudioObjectInstance::tryGetInputForChannel)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
                
    }
    
}
