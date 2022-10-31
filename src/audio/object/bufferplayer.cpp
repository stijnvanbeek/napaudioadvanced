/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "bufferplayer.h"

RTTI_BEGIN_CLASS(nap::audio::BufferPlayer)
    RTTI_PROPERTY("AutoPlay", &nap::audio::BufferPlayer::mAutoPlay, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Buffer", &nap::audio::BufferPlayer::mBufferResource, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::ParallelNodeObjectInstance<nap::audio::BufferPlayerNode>)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        bool BufferPlayer::initNode(int channel, BufferPlayerNode& node, utility::ErrorState& errorState)
        {
            if (mBufferResource != nullptr)
                node.setBuffer(mBufferResource->getBuffer());
            if (mAutoPlay)
                node.play(0, 1.);
            return true;
        }
                
    }
    
}
