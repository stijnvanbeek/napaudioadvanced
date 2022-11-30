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
         * This is a helper Node that can be used to manipulate when a node chain will be processed.
         * Normally a Node chain's processing will happen as soon as its output is pulled from an OutputPin.
         * A BufferNode however will not pull the OutputPin connected to its audioInput until the update() method is called.
         * Only after calling BufferNode::update() the audio buffer pulled from audioInput will be copied to audioOutput.
         */
        class NAPAPI BufferNode : public Node
        {
        public:
            BufferNode(NodeManager& nodeManager);
            
            InputPin audioInput = { this };
            OutputPin audioOutput = { this };

            /**
             * This method needs to be called to pull the latest audio buffer from audioInput and copy it to audioOutput.
             */
            void update();

        private:
            void bufferSizeChanged(int size) override;
            
            std::size_t mCacheSize = 0;
        };
        

        /**
         * This Process works together with the BufferNode.
         * When processed it calls BufferNode::update() on a number of registered BufferNodes.
         */
        class NAPAPI BufferUpdateProcess : public Process
        {
        public:
            BufferUpdateProcess(NodeManager& nodeManager) : Process(nodeManager) { }

            /**
             * Registers a BufferNode to be updated by this BufferUpdateProcess.
             * @param buffer The BufferNode to be registered
             */
            void registerBuffer(BufferNode* buffer);

            /**
             * Unregisters a BufferNode so it won't be updated by this BufferUpdateProcess any longer.
             * @param buffer The BufferNode to be unregistered
             */
            void unregisterBuffer(BufferNode* buffer);
            
        private:
            void process() override;

            std::vector<BufferNode*> mBuffers;
        };
        
    }
    
}
