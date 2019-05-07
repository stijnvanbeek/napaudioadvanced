#pragma once

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/multichannelobject.h>
#include <audio/node/mixnode.h>

namespace nap
{
    
    namespace audio
    {
        
        class Mixer : public MultiChannelObject
        {
            RTTI_ENABLE(MultiChannelObject)
            
        public:
            Mixer() = default;
            
            int mChannelCount = 1;
            std::vector<ResourcePtr<AudioObject>> mInputs;
            
        private:
            SafeOwner<Node> createNode(int channel, AudioService& service, utility::ErrorState& errorState) override
            {
                auto node = service.makeSafe<MixNode>(service.getNodeManager());
                for (auto& input : mInputs)
                    if (input != nullptr)
                    {
                        node->inputs.connect(*input->getInstance()->getOutputForChannel(channel % input->getInstance()->getChannelCount()));
                    }
                
                return std::move(node);
            }
            
            int getChannelCount() const override { return mChannelCount; }
        };
        
        
    }
    
}

