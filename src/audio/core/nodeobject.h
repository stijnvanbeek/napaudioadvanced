#pragma once

// Audio includes
#include <audio/core/audionode.h>
#include <audio/core/audioobject.h>

namespace nap
{
    
    namespace audio
    {
        
        template <typename NodeType>
        class NodeObject : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
            
        public:
            NodeObject() : AudioObject() { }
            
        private:
            virtual std::unique_ptr<AudioObjectInstance> createInstance();
        };
        
        
        template <typename NodeType>
        class NodeObjectInstance : public AudioObjectInstance, public NodeType
        {
            RTTI_ENABLE(AudioObjectInstance, NodeType)
            
        public:
            
            bool init(AudioService& service, utility::ErrorState& errorState) = 0;
            
        private:
        };
        
        
        template <typename NodeType>
        std::unique_ptr<AudioObjectInstance> NodeObject<NodeType>::createInstance()
        {
            return std::make_unique<NodeObjectInstance<NodeType>>();
        }
        
    }
    
}
