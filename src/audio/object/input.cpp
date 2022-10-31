/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "input.h"


RTTI_BEGIN_CLASS(nap::audio::Input)
    RTTI_PROPERTY("Channels", &nap::audio::Input::mChannels, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::ParallelNodeObjectInstance<nap::audio::InputNode>)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        
    }
    
}
