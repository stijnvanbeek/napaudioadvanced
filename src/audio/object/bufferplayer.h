/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/utility/safeptr.h>
#include <audio/core/nodeobject.h>
#include <audio/node/bufferplayernode.h>
#include <audio/resource/audiobufferresource.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * AudioObject to play back audio contained by an AudioBufferResource.
         */
        class NAPAPI BufferPlayer : public ParallelNodeObject<BufferPlayerNode>
        {
            RTTI_ENABLE(ParallelNodeObjectBase)
            
        public:
            BufferPlayer() = default;
            
            ResourcePtr<AudioBufferResource> mBufferResource = nullptr; ///< Property: 'BufferResource' Resource containing the buffer that will be played.
            bool mAutoPlay = true;                                      ///< Property: 'AutoPlay' If true, the object will start playing back immediately after initialization.
            
        private:
            bool initNode(int channel, BufferPlayerNode& node, utility::ErrorState& errorState) override;
        };


        /**
         * Instance of BufferPlayer
         */
        using BufferPlayerInstance = ParallelNodeObjectInstance<BufferPlayerNode>;
        
    }
    
}

