/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "mixer.h"

RTTI_BEGIN_CLASS(nap::audio::Mixer)
    RTTI_PROPERTY("Inputs", &nap::audio::Mixer::mInputs, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::ParallelNodeObjectInstance<nap::audio::MixNode>)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
    }
    
}
