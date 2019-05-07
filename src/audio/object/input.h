#pragma once

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/multichannelobject.h>
#include <audio/node/inputnode.h>
#include <audio/utility/safeptr.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Multichannel oscillator object.
         */
        class Input : public MultiChannelObject
        {
            RTTI_ENABLE(MultiChannelObject)
            
        public:
            std::vector<int> mChannels = { 0 }; ///< property: 'Channels' Defines what audio input channels to receive data from. The size of this array determines the number of channels that this component will output.
            
        private:
            SafeOwner<Node> createNode(int channel, AudioService& audioService, utility::ErrorState& errorState) override
            {
                SafeOwner<InputNode> node = audioService.makeSafe<InputNode>(audioService.getNodeManager());
                node->setInputChannel(mChannels[channel % mChannels.size()]);                
                return std::move(node);
            }
            
            int getChannelCount() const override { return mChannels.size(); }
        };
        
       
    }
    
}
