/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Nap includes
#include <component.h>
#include <componentptr.h>

// Audio includes
#include <audio/utility/safeptr.h>
#include <audio/component/audiocomponentbase.h>
#include <audio/node/circularbuffernode.h>

namespace nap
{
    
    namespace audio
    {
    
        class CircularBufferComponentInstance;
        

        /**
         * Component that manages circular buffers for a number of audio channels.
         * Audio input can be connected to be recorded by the circular buffers, in the resource as well as at runtime on instances.
         * At runtime the contents of the buffers can be read for each channel.
         */
        class NAPAPI CircularBufferComponent : public Component
        {
            RTTI_ENABLE(Component)
            DECLARE_COMPONENT(CircularBufferComponent, CircularBufferComponentInstance)
            
        public:
            CircularBufferComponent() : Component() { }
            
        public:
            // Properties
            nap::ComponentPtr<AudioComponentBase> mInput;    ///< Property: 'Input' The component whose audio output to rout to the circular buffer.
            
            std::vector<int> mChannelRouting = { 0 };        ///< Property: 'Routing' The size of this vector indicates the number of channels in the circular buffer.
                                                             ///< Each element in the array represents one channel of the circular buffer.
                                                             ///< The value of the element indicates the channel from the input that will be routed to the corresponding channel.

            int mBufferSize = 65536;                         ///< Property: 'BufferSize' The size of the circular buffers in samples.
            
        private:
        };


        /**
         * Instance of CircularBufferComponent
         */
        class NAPAPI CircularBufferComponentInstance : public ComponentInstance
        {
            RTTI_ENABLE(ComponentInstance)
        public:
            CircularBufferComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }
            
            // Inherited from ComponentInstance
            bool init(utility::ErrorState& errorState) override;

            /**
             * Request the circular buffer for a specific channel.
             * @param channel The index of the requested channel
             * @return Pointer to CircularBufferNode for the specified channel.
             */
            CircularBufferNode* getChannel(unsigned int channel);
            
        private:
            std::vector<SafeOwner<CircularBufferNode>> mNodes; // Circular buffer for each channel
            nap::ComponentInstancePtr<AudioComponentBase> mInput = { this, &CircularBufferComponent::mInput }; // Pointer to the component whose output will routed to the circular buffer.
        };
        
    }
        
}
