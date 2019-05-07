#pragma once

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/utility/safeptr.h>
#include <audio/core/multichannelobject.h>
#include <audio/node/circularbufferplayernode.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * AudioObject to play back audio contained by a CircularBufferComponent.
         */
        class CircularBufferPlayer : public MultiChannelObject
        {
            RTTI_ENABLE(MultiChannelObject)
            
        public:
            CircularBufferPlayer() = default;
            
            int mChannelCount = 1; ////< property: 'ChannelCount' Number of channels that will be played back from the source buffer */
            
        private:
            SafeOwner<Node> createNode(int channel, AudioService& audioService, utility::ErrorState& errorState) override;
            int getChannelCount() const override { return mChannelCount; }
        };
        
        
    }
    
}

