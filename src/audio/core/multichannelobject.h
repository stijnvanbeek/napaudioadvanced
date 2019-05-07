#pragma once

// Nap includes
#include <rtti/factory.h>
#include <nap/resource.h>

// Audio includes
#include <audio/core/audioobject.h>

namespace nap
{
    
    namespace audio
    {
        
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
            
            OutputPin* getOutputForChannel(int channel) override { return *mNodes[channel]->getOutputs().begin(); }
            int getChannelCount() const override { return mNodes.size(); }
            
            /**
             * If the managed nodes have (exactly) one input pin, this will be returned for the requested channel.
             */
            InputPinBase* getInputForChannel(int channel) override;
            
            /**
             * If the managed nodes have (exactly) one input pin, the number of channels will be returned, otherwise zero.
             */
            int getInputChannelCount() const override;
            
            /**
             * Returns a raw pointer to the DSP node for the specified channel.
             * Returns nullptr if the given channel is out of bounds
             */
            template <typename T>
            T* getChannel(unsigned int channel) { return rtti_cast<T>(getChannelNonTyped(channel)); }
            
            Node* getChannelNonTyped(unsigned int channel);
            
            
            bool resize(unsigned int channelCount);
            
        private:
            std::vector<SafeOwner<Node>> mNodes;
            AudioService* mService  = nullptr;
        };
                
    }
        
}


