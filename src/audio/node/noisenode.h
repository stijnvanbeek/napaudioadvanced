/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <audio/core/audionode.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * White noise generator
         */
        class NAPAPI NoiseNode : public Node
        {
        public:
            NoiseNode(NodeManager& manager) : Node(manager) { }
        
            /**
             * Output signal containing the noise
             */
            OutputPin audioOutput = { this };
            
        private:
            void process() override;
        };
        
    }
}
