#pragma once

// Nap includes
#include <rtti/factory.h>
#include <nap/resource.h>
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/audioobject.h>

namespace nap
{
    
    namespace audio
    {
        
        class NAPAPI MultiChannel : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
            
        public:
            MultiChannel() = default;
            
            std::unique_ptr<AudioObjectInstance> createInstance() override;
            
            ResourcePtr<AudioObject> mChannel = nullptr;
            int mChannelCount = 1; ///< Property: 'ChannelCount' The number of channels
            
        private:
        };
        
        
        class NAPAPI MultiChannelInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
            
        public:
            MultiChannelInstance(MultiChannel& resource) : AudioObjectInstance(resource) { }
            
            bool init(AudioService& service, utility::ErrorState& errorState) override;
            
            /**
             * Returns a raw pointer to the DSP node for the specified channel.
             * Returns nullptr if the given channel is out of bounds
             */
            template <typename T>
            T* getChannel(unsigned int channel) { return rtti_cast<T>(getChannelNonTyped(channel)); }
            
            AudioObjectInstance* getChannelNonTyped(unsigned int channel) { return channel < mChannels.size() ? mChannels[channel].get() : nullptr; }
            
            // Inherited from AudioObjectInstance
            OutputPin* getOutputForChannel(int channel) override { return mChannels[channel]->getOutputForChannel(0); }
            int getChannelCount() const override { return mChannels.size(); }
            void connect(unsigned int channel, OutputPin& pin) override { mChannels[channel]->connect(0, pin); }
            int getInputChannelCount() const override { return (mChannels[0]->getInputChannelCount() == 1) ? mChannels.size() : 0; }
            
        private:
            std::vector<std::unique_ptr<AudioObjectInstance>> mChannels;
        };
        
        
        /**
         * Base class for audio objects that contain a number of nodes of the same type, typically for performing multichannel processing.
         */
        class NAPAPI MultiChannelObject : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
            
            friend class MultiChannelObjectInstance;
            
        public:
            MultiChannelObject() = default;
            
            std::unique_ptr<AudioObjectInstance> createInstance() override;
            
        private:
            /**
             * This factory method has to be implemented by descendants to create a DSP Node for a certain channel.
             */
            virtual SafeOwner<Node> createNode(int channel, AudioService& audioService, utility::ErrorState& errorState) = 0;
            
            /**
             * This method has to be overwritten by descendants to return the number of nodes/channels that the instance of this object will own.
             */
            virtual int getChannelCount() const = 0;
        };

        
        
        /**
         * Instance of a MultiChannelObject. In most cases only the MultiChannelObject has to be overwritten to create your own MultiChannelObject type while this instance class can be left untouched.
         */
        class NAPAPI MultiChannelObjectInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
            
        public:
            MultiChannelObjectInstance(MultiChannelObject& resource) : AudioObjectInstance(resource) { }
            
            bool init(AudioService& service, utility::ErrorState& errorState) override;
            
            /**
             * Disposes all of the old nodes and their connections and creates @channelCount new ones.
             */
            bool create(unsigned int channelCount);
            
            /**
             * Returns a raw pointer to the DSP node for the specified channel.
             * Returns nullptr if the given channel is out of bounds
             */
            template <typename T>
            T* getChannel(unsigned int channel) { return rtti_cast<T>(getChannelNonTyped(channel)); }
            
            Node* getChannelNonTyped(unsigned int channel);            

            // Inherited from AudioObjectInstance
            OutputPin* getOutputForChannel(int channel) override { return *mNodes[channel]->getOutputs().begin(); }
            int getChannelCount() const override { return mNodes.size(); }
            void connect(unsigned int channel, OutputPin& pin) override { (*mNodes[channel]->getInputs().begin())->connect(pin); }
            int getInputChannelCount() const override { return mNodes.size(); }

        protected:
            std::vector<SafeOwner<Node>> mNodes;

        private:
            virtual bool initNode(Node& newNode, utility::ErrorState& errorState) { return true; }
            
            AudioService* mService  = nullptr;
        };
        
        
    }
        
}


