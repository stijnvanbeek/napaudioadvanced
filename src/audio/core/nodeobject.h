#pragma once

// Audio includes
#include <audio/core/audionode.h>
#include <audio/core/audioobject.h>

namespace nap
{
    
    namespace audio
    {

        template <typename NodeType> class NodeObjectInstance;


        template <typename NodeType>
        class NodeObject : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
            
        public:
            NodeObject() : AudioObject() { }
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance(AudioService& service, utility::ErrorState& errorState) override
            {
                auto result = std::make_unique<NodeObjectInstance<NodeType>>(service);
                return std::move(result);
            }
        };
        

        template <typename NodeType>
        class NodeObjectInstance : public AudioObjectInstance, SafeOwner<NodeType>
        {
            RTTI_ENABLE(AudioObjectInstance)
            
        public:
            NodeObjectInstance(AudioService& service) : AudioObjectInstance(), SafeOwner<NodeType>(service.makeSafe<NodeType>(service.getNodeManager())) { }

            NodeObjectInstance(AudioService& service, const std::string& name) : AudioObjectInstance(name), SafeOwner<NodeType>(service.makeSafe<NodeType>(service.getNodeManager())) { }

            // Inherited from AudioObjectInstance
            OutputPin* getOutputForChannel(int channel) override;
            int getChannelCount() const override { return this->getOutputs().size();; }
            void connect(unsigned int channel, OutputPin& pin) override;
            int getInputChannelCount() const override { return this->getInputs().size(); }
        };


        template <typename NodeType>
        OutputPin* NodeObjectInstance<NodeType>::getOutputForChannel(int channel)
        {
            auto i = 0;
            for (auto& output : this->getOutputs())
            {
                if (i == channel)
                    return output;
                i++;
            }
            return nullptr;
        }


        template <typename NodeType>
        void NodeObjectInstance<NodeType>::connect(unsigned int channel, OutputPin& pin)
        {
            auto i = 0;
            for (auto& input : this->getInputs())
            {
                if (i == channel)
                {
                    input->connect(pin);
                    return;
                }
                i++;
            }
        }
        
    }
    
}
