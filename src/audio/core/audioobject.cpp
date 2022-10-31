/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "audioobject.h"

// Nap includes
#include <nap/logger.h>

// RTTI
RTTI_DEFINE_BASE(nap::audio::AudioObject)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AudioObjectInstance)
    RTTI_FUNCTION("getChannelCount", &nap::audio::AudioObjectInstance::getChannelCount)
    RTTI_FUNCTION("getOutputForChannel", &nap::audio::AudioObjectInstance::tryGetOutputForChannel)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
                
    }
    
}
