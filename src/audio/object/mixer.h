#pragma once

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/nodeobject.h>
#include <audio/node/mixnode.h>

namespace nap
{
    
    namespace audio
    {
        
        class Mixer : public MultiChannel<MixNode>
        {
            RTTI_ENABLE(MultiChannel<MixNode>)
            
        public:
            Mixer() = default;
            
            int mChannelCount = 1;
            std::vector<ResourcePtr<AudioObject>> mInputs;
            
        private:
            bool initNode(int channel, MixNode& node, utility::ErrorState& errorState) override
            {
                for (auto& input : mInputs)
                    if (input != nullptr)
                    {
                        node.inputs.connect(*input->getInstance()->getOutputForChannel(channel % input->getInstance()->getChannelCount()));
                    }
                return true;
            }
        };
        
        
    }
    
}

