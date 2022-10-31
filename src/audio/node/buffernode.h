/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <audio/core/audionode.h>

namespace nap
{

    namespace audio
    {
        
        class NAPAPI BufferNode : public Node
        {
        public:
            BufferNode(NodeManager& nodeManager);
            
            InputPin audioInput = { this };
            OutputPin audioOutput = { this };
            
            void update();

        private:
            void bufferSizeChanged(int size) override;
            
            std::size_t mCacheSize = 0;
        };
        
        
        class NAPAPI BufferUpdateProcess : public Process
        {
        public:
            BufferUpdateProcess(NodeManager& nodeManager) : Process(nodeManager) { }
            
            void process() override;
            
            void registerBuffer(BufferNode*);
            void unregisterBuffer(BufferNode*);
            
        private:
            std::vector<BufferNode*> mBuffers;
        };
        
    }
    
}
