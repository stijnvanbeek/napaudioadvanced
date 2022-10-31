/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
        
        class NAPAPI Mixer : public ParallelNodeObject<MixNode>
        {
            RTTI_ENABLE(ParallelNodeObjectBase)
            
        public:
            Mixer() = default;
            
            std::vector<ResourcePtr<AudioObject>> mInputs;
            
        private:
            bool initNode(int channel, MixNode& node, utility::ErrorState& errorState) override
            {
                node.inputs.reserveInputs(mInputs.size());

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

