#include "chain.h"

RTTI_BEGIN_CLASS(nap::audio::Chain)
    RTTI_PROPERTY("Objects", &nap::audio::Chain::mObjects, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("Input", &nap::audio::Chain::mInput, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::ChainInstance)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::audio::MultiChainBase)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::audio::MultiChain)
    RTTI_PROPERTY("ChainCount", &nap::audio::MultiChain::mChainCount, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("IsActive", &nap::audio::MultiChain::mIsActive, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::MultiChainInstance)
RTTI_END_CLASS

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
            
            if (resource->mObjects.empty())
            {
                errorState.fail("Chain can not initialize empty: %s", resource->mID.c_str());
                return false;
            }
            
            for (auto& objectResource : resource->mObjects)
            {
                auto newObject = instantiateObjectInChain(*objectResource, audioService, errorState);
                if (newObject == nullptr)
                    return false;
                if (!mObjects.empty())
                {
                    auto previous = mObjects.back().get();
                    if (!connectObjectsInChain(*previous, *newObject, errorState))
                        return false;
                }
                mObjects.emplace_back(std::move(newObject));
            }
            
            if (resource->mInput != nullptr)
            {
                mObjects[0]->connect(*resource->mInput->getInstance());
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


        std::unique_ptr<AudioObjectInstance> ChainInstance::instantiateObjectInChain(AudioObject& resource, AudioService& audioService, utility::ErrorState& errorState)
        {
            auto newObject = resource.instantiate<AudioObjectInstance>(audioService, errorState);
            if (newObject == nullptr)
            {
                errorState.fail("Failed to create object in chain: %s", resource.mID.c_str());
                return nullptr;
            }
            return newObject;
        }
        
        
        std::unique_ptr<AudioObjectInstance> MultiChainBase::createInstance()
        {
            return std::make_unique<MultiChainInstance>(*this);
        }

        
        std::unique_ptr<AudioObjectInstance> MultiChainInstance::instantiateObjectInChain(AudioObject& resource, AudioService& audioService, utility::ErrorState& errorState)
        {
            auto chainResource = getResource<MultiChainBase>();
            auto multiResource = rtti_cast<MultiObjectBase>(&resource);
            
            if (multiResource == nullptr)
            {
                errorState.fail("Object inside MultiChain is not a MultiObject: %s", resource.mID.c_str());
                return nullptr;
            }
            
            // InstanceCount and IsActive properties of the MultiObjects in the chain are overwritten by properties of the chain itself.
            multiResource->mInstanceCount = chainResource->mChainCount;
            multiResource->mIsActive = chainResource->mIsActive;
            std::unique_ptr<MultiObjectInstance> newObject = resource.instantiate<MultiObjectInstance>(audioService, errorState);
            if (newObject == nullptr)
            {
                errorState.fail("Failed to create object in chain: %s", resource.mID.c_str());
                return nullptr;
            }
            return newObject;
        }
        
        
        bool MultiChainInstance::connectObjectsInChain(AudioObjectInstance& source, AudioObjectInstance& destination, utility::ErrorState& errorState)
        {
            auto sourceMulti = rtti_cast<MultiObjectInstance>(&source);
            auto destinationMulti = rtti_cast<MultiObjectInstance>(&destination);
            destinationMulti->connect(*sourceMulti);
            return true;
        }
        
        
        bool MultiChainInstance::addChain(utility::ErrorState& errorState)
        {
            AudioObjectInstance* previous = nullptr;
            for (auto i = 0; i < getObjectCount(); ++i)
            {
                auto multi = getObject<MultiObjectInstance>(i);
                auto newObject = multi->addObjectNonTyped(errorState);
                if (newObject == nullptr)
                {
                    errorState.fail("Unable to create now object %s", getResource().mID.c_str());
                    return false;
                }
                if (previous != nullptr)
                    newObject->connect(*previous);
                previous = newObject;
            }
            return true;
        }
        
        
        void MultiChainInstance::setChainActive(int index, bool isActive)
        {
            back<MultiObjectInstance>()->setActive(back<MultiObjectInstance>()->getObjectNonTyped(index), isActive);
        }
        
        
    }
    
}
