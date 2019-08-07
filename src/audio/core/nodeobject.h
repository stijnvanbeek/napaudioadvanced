#pragma once

// Audio includes
#include <audio/core/audionode.h>
#include <audio/core/audioobject.h>

namespace nap
{
    
    namespace audio
    {
        
        class NodeObject : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
            
        public:
            NodeObject() : AudioObject() { }
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance(AudioService& service, utility::ErrorState& errorState) override;
            
            virtual SafeOwner<Node> createNode(AudioService& service) = 0;
        };
        
        
        class NodeObjectInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
            
        public:
            NodeObjectInstance(SafeOwner<Node> node) : AudioObjectInstance(), mNode(std::move(node)) { }
            NodeObjectInstance(const std::string& name, SafeOwner<Node> node) : AudioObjectInstance(name), mNode(std::move(node)) { }
            
            template <typename T>
            T* getNode() { return rtti_cast<T>(getNodeNonTyped()); }
            
            Node* getNodeNonTyped() { return mNode.getRaw(); }
            
            // Inherited from AudioObjectInstance
            OutputPin* getOutputForChannel(int channel) override;
            int getChannelCount() const override { return mNode->getOutputs().size();; }
            void connect(unsigned int channel, OutputPin& pin) override;
            int getInputChannelCount() const override { return mNode->getInputs().size(); }
            
        private:
            SafeOwner<Node> mNode = nullptr;
        };
        
        
    }
    
}
