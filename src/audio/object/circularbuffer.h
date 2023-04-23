/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/utility/safeptr.h>
#include <audio/core/audioobject.h>
#include <audio/node/circularbuffernode.h>

namespace nap
{
    
    namespace audio
    {
    
        class CircularBufferInstance;
        

        /**
         * Object that reads its input into a circular buffer that can be read from at requested positions relative to the input position.
         */
        class NAPAPI CircularBuffer : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
            
        public:
            CircularBuffer() : AudioObject() { }
            
        public:
            // Properties
            ResourcePtr<AudioObject> mInput;          ///< Property: 'Input' The object whose audio output to rout to the circular buffer.
            
            std::vector<int> mChannelRouting = { 0 }; ///< Property: 'ChannelRouting' The size of this vector indicates the number of channels in the circular buffer.
            ///< Each element in the array represents one channel of the circular buffer.
            ///< The value of the element indicates the channel from the input that will be routed to the corresponding channel.

            bool mRootProcess = true; ///< Property: 'RootProcess' Indicates if the CircularBufferNOde objects are added as root process to the NodeManager
            int mBufferSize = 65536;  ///< Property: 'BufferSize' The size of the circular buffer in samples
            
        private:
            // Inherited from AudioObject
            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;
        };


        /**
         * Instance of CircularBufferInstance
         */
        class NAPAPI CircularBufferInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
        public:
            CircularBufferInstance() : AudioObjectInstance() { }
            CircularBufferInstance(const std::string& name) : AudioObjectInstance(name) { }

            /**
             * Initializes the CircularBufferInstance with an input
             * @param input AudioObject that will produce input or the circular buffers
             * @param channelRouting For each channel of the circular buffer this indicates the input channel of input.
             * @param rootProcess True if the circular buffers should be root processes of the NodeManager. This means processing will happen automatically.
             * @param bufferSize The size of the circular buffers in samples
             * @param nodeManager The NodeManager the circular buffers will be processed on
             * @param errorState Logs errors during initialization
             * @return True on success
             */
            bool init(AudioObjectInstance& input, const std::vector<int>& channelRouting, bool rootProcess, int bufferSize, NodeManager& nodeManager, utility::ErrorState& errorState);

            /**
             * Initializes the CircularBufferInstance without input
             * @param channelCount Number of channels of the circular buffer
             * @param rootProcess True if the circular buffers should be root processes of the NodeManager. This means processing will happen automatically.
             * @param bufferSize The size of the circular buffers in samples
             * @param nodeManager The NodeManager the circular buffers will be processed on
             * @param errorState Logs errors during initialization
             * @return True on success
             */
            bool init(int channelCount, bool rootProcess, int bufferSize, NodeManager& nodeManager, utility::ErrorState& errorState);

            /**
             * Request a channel node of the circular buffer.
             * @param channel Index of the requested channel
             * @return Pointer to CircularBufferNode for the specified channel
             */
            SafePtr<CircularBufferNode> getChannel(unsigned int channel);

            /**
             * @return Number of channels of this object
             */
			int getBufferChannelCount() const { return mNodes.size(); }

			/**
			 * Clears the contents of each channel of the circular buffer.
			 */
			void clear();

        private:
			// Inherited from AudioObjectInstance
			OutputPin* getOutputForChannel(int channel) override { return nullptr; }
			int getChannelCount() const override { return 0; }

			std::vector<SafeOwner<CircularBufferNode>> mNodes; // Circular buffer for each channel
        };
        
    }
        
}
