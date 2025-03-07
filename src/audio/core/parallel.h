/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Nap includes
#include <rtti/factory.h>
#include <nap/resource.h>
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/audioobject.h>
#include <audio/core/nodeobject.h>

namespace nap
{
    
    namespace audio
    {

        /**
         * Multi channel AudioObject that wraps a mono AudioObject to process each channel.
         */
        class NAPAPI Parallel : public AudioObject {
            RTTI_ENABLE(AudioObject)
        public:
            std::vector<ResourcePtr<AudioObject>> mChannels; ///< Property: 'Channels' Mono AudioObject resource to process each channel of the Parallel object.
            ResourcePtr<AudioObject> mInput = nullptr;       ///< Property: 'Input' Link to AudioObject resource that provides audio input for this Parallel object
            int mChannelCount = 1;                           ///< Property: 'ChannelCount' Number of channels processed by this object

        private:
            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;
        };


        /**
         * Instance of Parallel resource.
         * Multi channel AudioObject that wraps a mono AudioObject to process each channel.
         */
        class NAPAPI ParallelInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
        public:
            ParallelInstance() = default;
            ParallelInstance(const std::string& name) : AudioObjectInstance(name) { }

            /**
             * Adds a processing channel at runtime.
             * @param resource AudioObject resource for the new processing channel
             * @param nodeManager NodeManager that this ParallelInstance is processed on.
             * @param errorState Logs error during the instantiation of the resource.
             * @return True on success
             */
            bool addChannel(AudioObject& resource, NodeManager& nodeManager, utility::ErrorState& errorState);

            /**
             * Add a mono audio object instance as a new channel of the object at runtime.
             * @param channel
             */
            void addChannel(std::unique_ptr<AudioObjectInstance> channel);

            /**
             * Returns a channel as mono AudioObjectInstance descendant
             * @tparam T Type of the requested AudioObjectInstance
             * @param channel Channel for which the processing AudioObjectInstance is requested.
             * @return Pointer to the AudioObjectInstance that performs processing for the given channel. Nullptr if the type T does not match.
             */
            template <typename T>
            T* getChannel(int channel) { return rtti_cast<T>(mChannels[channel].get()); }

            /**
             * Returns a channel as mono AudioObjectInstance descendant
             * @param channel Channel for which the processing AudioObjectInstance is requested.
             * @return Pointer to the AudioObjectInstance that performs processing for the given channel. Nullptr if channel is out of bounds.
             */
            AudioObjectInstance* getChannelNonTyped(int channel);

            // Inherited from AudioObjectInstance
            audio::OutputPin* getOutputForChannel(int channel) override { return mChannels[channel]->getOutputForChannel(0); }
            int getChannelCount() const override { return mChannels.size(); }
            void connect(unsigned int channel, audio::OutputPin& pin) override { mChannels[channel]->connect(0, pin); }
			void disconnect(unsigned int channel, audio::OutputPin& pin) override { mChannels[channel]->disconnect(0, pin); }
            int getInputChannelCount() const override { return (mChannels[0]->getInputChannelCount() == 1) ? mChannels.size() : 0; }

        protected:
            std::vector<std::unique_ptr<AudioObjectInstance>> mChannels;

        };


    }
        
}


