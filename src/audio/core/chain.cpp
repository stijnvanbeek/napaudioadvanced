#include "chain.h"

// Audio includes
#include <audio/core/multiobject.h>

namespace nap
{
    
    namespace audio
    {
        
        std::unique_ptr<AudioObjectInstance> Chain::createInstance()
        {
            return std::make_unique<ChainInstance>(*this);
        }
        
        
        bool ChainInstance::init(AudioService& audioService, utility::ErrorState& errorState)
        {
            auto resource = getResource<Chain>();
            
            for (auto& objectResource : resource->mObjects)
            {
                auto newObject = objectResource->instantiate<AudioObjectInstance>(audioService, errorState);
                if (newObject == nullptr)
                {
                    errorState.fail("Failed to create object in chain: %s", objectResource->mID.c_str());
                    return false;
                }
                if (!newObject->init(audioService, errorState))
                {
                    errorState.fail("Failed to initialize object in chain: %s", objectResource->mID.c_str());
                    return false;
                }
                if (!mObjects.empty())
                {
                    auto previous = mObjects.back().get();
                    if (!connectObjectsInChain(*previous, *newObject, errorState))
                        return false;
                }
                mObjects.emplace_back(std::move(newObject));
            }
            
            return true;
        }
        
        
        AudioObjectInstance* ChainInstance::getObjectNonTyped(unsigned int index)
        {
            if (index < mObjects.size())
                return mObjects[index].get();
            else
                return nullptr;
        }
        
        
        bool ChainInstance::connectObjectsInChain(AudioObjectInstance& source, AudioObjectInstance& destination, utility::ErrorState& errorState)
        {
            auto destinationInput = dynamic_cast<IMultiChannelInput*>(&destination);
            if (destinationInput == nullptr)
            {
                errorState.fail("Object in chain has no input: %s", destination.getResource().mID.c_str());
                return false;
            }
            destinationInput->connect(source);
            return true;
        }


        
        std::unique_ptr<AudioObjectInstance> EffectChain::createInstance()
        {
            return std::make_unique<EffectChainInstance>(*this);
        }


        bool EffectChainInstance::init(AudioService& audioService, utility::ErrorState& errorState)
        {
            if (!ChainInstance::init(audioService, errorState))
                return false;
            
            auto first = getObjectNonTyped(0);
            mInput = dynamic_cast<IMultiChannelInput*>(first);
            if (mInput == nullptr)
            {
                errorState.fail("First object in chain has no input: %s", first->getResource().mID.c_str());
                return false;
            }
            
            return true;
        }
        
        
        std::unique_ptr<AudioObjectInstance> MultiEffectChain::createInstance()
        {
            return std::make_unique<MultiEffectChainInstance>(*this);
        }

        
        bool MultiEffectChainInstance::connectObjectsInChain(AudioObjectInstance& source, AudioObjectInstance& destination, utility::ErrorState& errorState)
        {
            auto sourceMulti = rtti_cast<MultiEffectInstance>(&source);
            if (sourceMulti == nullptr)
            {
                errorState.fail("Object in MultiEffectChain is no MultiObject: %s", sourceMulti->getResource().mID.c_str());
                return false;
            }
            auto destinationMulti = rtti_cast<MultiEffectInstance>(&destination);
            if (destinationMulti == nullptr)
            {
                errorState.fail("Object in MultiEffectChain is no MultiObject: %s", destinationMulti->getResource().mID.c_str());
                return false;
            }
            destinationMulti->connect(*sourceMulti);
            return true;
        }

    }
    
}
