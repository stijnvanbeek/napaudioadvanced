#include "multichannelobject.h"


// Nap includes
#include <entity.h>
#include <nap/core.h>
#include <nap/logger.h>

// Audio includes
#include <audio/service/audioservice.h>


// RTTI
RTTI_DEFINE_BASE(nap::audio::MultiChannelObject)

RTTI_BEGIN_CLASS(nap::audio::MultiChannelObjectInstance)
    RTTI_FUNCTION("getChannel", &nap::audio::MultiChannelObjectInstance::getChannelNonTyped)
    RTTI_FUNCTION("create", &nap::audio::MultiChannelObjectInstance::create)
RTTI_END_CLASS


namespace nap
{
    
    namespace audio
    {
        
        std::unique_ptr<AudioObjectInstance> MultiChannel::createInstance(AudioService& service, utility::ErrorState& errorState)
        {
            auto instance = std::make_unique<MultiChannelInstance>();
            if (!instance->init(*mChannel, mChannelCount, service, errorState))
                return nullptr;
            
            return std::move(instance);
        }

        
        bool MultiChannelInstance::init(AudioObject& channelResource, int channelCount, AudioService& service, utility::ErrorState& errorState)
        {
            for (auto channel = 0; channel < channelCount; ++channel)
            {
                auto channelInstance = channelResource.instantiate<AudioObjectInstance>(service, errorState);
                if (channelInstance == nullptr)
                {
                    errorState.fail("Failed to instantiate channel %s for %s", channelResource.mID.c_str(), getName().c_str());
                    return false;
                }
                mChannels.emplace_back(std::move(channelInstance));
            }
            
            return true;
        }
        
        
        std::unique_ptr<AudioObjectInstance> MultiChannelObject::createInstance(AudioService& service, utility::ErrorState& errorState)
        {
            auto instance = std::make_unique<MultiChannelObjectInstance>();
            MultiChannelObjectInstance::NodeFactory nodeFactory = [&](int channel, AudioService& audioService, utility::ErrorState& aErrorState)
            {
                return createNode(channel, audioService, aErrorState);
            };
            
            if (!instance->init(nodeFactory, getChannelCount(), service, errorState))
                return nullptr;
            
            return std::move(instance);
        }
        
        
        bool MultiChannelObjectInstance::init(NodeFactory nodeFactory, int channelCount, AudioService& service, utility::ErrorState& errorState)
        {
            mService = &service;
            mNodeFactory = nodeFactory;
            for (auto channel = 0; channel < channelCount; ++channel)
            {
                auto node = nodeFactory(channel, service, errorState);
                if (!errorState.check(node != nullptr, "Error creating node in %s", getName().c_str()))
                    return false;
                
                if (!errorState.check(node->getOutputs().size() == 1, "Nodes in %s have to be mono", getName().c_str()))
                    return false;
                
                if (initNode(*node, errorState) == false)
                {
                    errorState.fail("Failed to init node.");
                    return false;
                }
                
                mNodes.emplace_back(std::move(node));
            }
            return true;
        }
        
        
        Node* MultiChannelObjectInstance::getChannelNonTyped(unsigned int channel)
        {
            if (channel < mNodes.size())
                return mNodes[channel].getRaw();
            else
                return nullptr;
        }


        bool MultiChannelObjectInstance::create(unsigned int channelCount)
        {
            mNodes.clear();
            
            for (auto channel = 0; channel < channelCount; ++channel)
            {
                utility::ErrorState errorState;
                auto node = mNodeFactory(channel, *mService, errorState);
                if (node == nullptr)
                {
                    nap::Logger::warn("Failed to resize MultiChannelObjectInstance %s", getName().c_str());
                    return false;
                }
                
                if (!(node->getOutputs().size() == 1))
                {
                    nap::Logger::warn("Failed to resize MultiChannelObjectInstance %s: node is not mono", getName().c_str());
                    return false;
                }
                
                if (initNode(*node, errorState) == false)
                {
                    nap::Logger::warn("Failed to resize MultiChannelObjectInstance %s: failed to init node.", getName().c_str());
                    return false;
                }
                
                mNodes.emplace_back(std::move(node));
            }

            return true;
        }
        
        
    }
    
}
