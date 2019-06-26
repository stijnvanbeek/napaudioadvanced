#include "nodeobject.h"

namespace nap
{
    
    namespace audio
    {
        
        std::unique_ptr<AudioObjectInstance> NodeObject::createInstance(AudioService& service, utility::ErrorState& errorState)
        {
            auto result = std::make_unique<NodeObjectInstance>(createNode(service));
            return result;
        }

        
        OutputPin* NodeObjectInstance::getOutputForChannel(int channel)
        {
            auto i = 0;
            for (auto& output : mNode->getOutputs())
            {
                if (i == channel)
                    return output;
                i++;
            }
            return nullptr;
        }
        
        
        void NodeObjectInstance::connect(unsigned int channel, OutputPin& pin)
        {
            auto i = 0;
            for (auto& input : mNode->getInputs())
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
