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

        /**
         * AudioObject containing a MixNode for each channel
         */
        class NAPAPI Mixer : public ParallelNodeObject<MixNode>
        {
            RTTI_ENABLE(ParallelNodeObjectBase)
            
        public:
            Mixer() = default;
            
            std::vector<ResourcePtr<AudioObject>> mInputs; ///< Property: 'Inputs' AudioObjects whose output will be mixed by this object
            
        private:
            bool initNode(int channel, MixNode& node, utility::ErrorState& errorState) override;
        };
        
        
    }
    
}

