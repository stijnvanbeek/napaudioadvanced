/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "nodeobject.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::NodeObjectInstanceBase)
	RTTI_FUNCTION("get", &nap::audio::NodeObjectInstanceBase::getNonTyped)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::ParallelNodeObjectBase)
    RTTI_PROPERTY("ChannelCount", &nap::audio::ParallelNodeObjectBase::mChannelCount, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::ParallelNodeObjectInstanceBase)
    RTTI_FUNCTION("getChannel", &nap::audio::ParallelNodeObjectInstanceBase::getChannelNonTyped)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {



    }
    
}
