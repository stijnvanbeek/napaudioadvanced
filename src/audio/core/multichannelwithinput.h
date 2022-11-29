/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <nap/resource.h>

#include <audio/core/nodeobject.h>


namespace nap
{
    namespace audio
    {
        
        /**
         * This class exists so we can RTTI define the 'input' ResourcePtr of MultiChannelWithInput.
         */
        class NAPAPI InputResourceContainer
        {
            RTTI_ENABLE()
        public:
            InputResourceContainer() = default;
			
            virtual ~InputResourceContainer() = default;
            
            ResourcePtr<AudioObject> mInput = nullptr; ///< Property: 'Input' AudioObject that generates the input for the object that owns the InputResourceContainer.
        };


         /**
          * MultiChannel AudioObject with a single input property baked in.
          * @tparam NodeType Each channel of the instance is processed by a (mono) node of type NodeType.
          */
        template <typename NodeType>
        class NAPAPI MultiChannelWithInput : public ParallelNodeObject<NodeType>, public InputResourceContainer
        {
            RTTI_ENABLE(ParallelNodeObjectBase, InputResourceContainer)
            
        public:
            MultiChannelWithInput() = default;

        private:
             /**
              * Overwritten from ParallelNodeObject base class.
              * Handles connection of the input to the node of a specific channel, and calls onInitNode().
              */
            virtual bool initNode(int channel, NodeType& node, utility::ErrorState& errorState) override final
            {
                // mInput is allowed to stay nullptr, then the input will simply not be connected.
                if(mInput != nullptr)
                    (*node.getInputs().begin())->connect(*mInput->getInstance()->getOutputForChannel(channel % mInput->getInstance()->getChannelCount()));
                
                return onInitNode(channel, node, errorState);
            }
            
            /**
             * This method needs to be overwritten by descendants if specific node initialization behaviour is needed for NodeType.
             * @param channel The channel for which the node performs processing.
             * @param node The node being initialized.
             * @param errorState Logs errors during the initialization process.
             * @return True on success.
             */
            virtual bool onInitNode(int channel, NodeType& node, utility::ErrorState& errorState) { return true; };
        };
        
    }
}
