#include "multiobject.h"

// Nap includes
#include <entity.h>
#include <nap/logger.h>

// Audio includes
#include <audio/core/audioobject.h>


// RTTI
RTTI_BEGIN_CLASS(nap::audio::MultiObjectBase)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::audio::MultiObject)
    RTTI_PROPERTY("Object", &nap::audio::MultiObject::mObject, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("InstanceCount", &nap::audio::MultiObject::mInstanceCount, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("IsActive", &nap::audio::MultiObject::mIsActive, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::MultiObjectInstance)
    RTTI_CONSTRUCTOR(nap::audio::MultiObjectBase&)
    RTTI_FUNCTION("getObject", &nap::audio::MultiObjectInstance::getObjectNonTyped)
    RTTI_FUNCTION("getObjectCount", &nap::audio::MultiObjectInstance::getObjectCount)
    RTTI_FUNCTION("setActive", &nap::audio::MultiObjectInstance::setActive)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        

        std::unique_ptr<AudioObjectInstance> MultiObjectBase::createInstance()
        {
            return std::make_unique<MultiObjectInstance>(*this);
        }
        
        
        bool MultiObjectInstance::init(AudioService& audioService, utility::ErrorState& errorState)
        {
            mAudioService = &audioService;
            
            MultiObjectBase* resource = rtti_cast<MultiObjectBase>(&getResource());
            
            // Instantiate the objects.
            AudioObject* objectResource = getObjectResource();
            if (objectResource == nullptr)
                return false;
            
            for (auto i = 0; i < resource->mInstanceCount; ++i)
            {
                auto instance = objectResource->instantiate<AudioObjectInstance>(audioService, errorState);
                if (instance == nullptr)
                    return false;
                mObjects.emplace_back(std::move(instance));
            }
            
            // Determine the channel count
            int channelCount = 0;
            if (mObjects.empty())
            {
                // Create a dummy object instance to acquire the channel count..
                auto instance = objectResource->instantiate<AudioObjectInstance>(audioService, errorState);
                if (instance == nullptr)
                    return false;
                channelCount = instance->getChannelCount();
            }
            else
                channelCount = mObjects[0]->getChannelCount();
            
            // Create the output mixers
            for (auto channel = 0; channel < channelCount; ++channel)
            {
                // we dont connect anything yet
                mMixers.emplace_back(mAudioService->makeSafe<MixNode>(mAudioService->getNodeManager()));
            }
            
            // Connect the objects that are active
            for (auto i = 0; i < mObjects.size(); ++i)
            {
                if (resource->mIsActive)
                {
                    auto object = mObjects[i].get();
                    for (auto channel = 0; channel < mMixers.size(); ++channel)
                        mMixers[channel]->inputs.connect(*object->getOutputForChannel(channel));
                }
            }

            return true;
        }

        
        AudioObjectInstance* MultiObjectInstance::getObjectNonTyped(unsigned int index)
        {
            if (index < mObjects.size())
                return mObjects[index].get();
            
            nap::Logger::warn("MultiObjectInstance %s: index for getObject() out of bounds", mID.c_str());
            return nullptr;
        }
        
        
        AudioObjectInstance* MultiObjectInstance::addObjectNonTyped(utility::ErrorState& errorState)
        {
            auto objectResource = getObjectResource();
            if (objectResource == nullptr)
                return nullptr;
            auto instance = objectResource->instantiate<AudioObjectInstance>(*mAudioService, errorState);
            if (instance == nullptr)
                return nullptr;
            auto result = instance.get();
            mObjects.emplace_back(std::move(instance));
            return result;
        }
        
        
        bool MultiObjectInstance::setActive(AudioObjectInstance* object, bool isActive)
        {
            for (auto& element : mObjects)
                if (element.get() == object)
                {
                    if (isActive)
                        for (auto channel = 0; channel < mMixers.size(); ++channel)
                            mMixers[channel]->inputs.enqueueConnect(*object->getOutputForChannel(channel));
                    else
                        for (auto channel = 0; channel < mMixers.size(); ++channel)
                            mMixers[channel]->inputs.enqueueDisconnect(*object->getOutputForChannel(channel));
                    
                    return true;
                }
            return false;
        }

        
        
        OutputPin* MultiObjectInstance::getOutputForChannel(int channel)
        {
            return &mMixers[channel]->audioOutput;
        }
        
        
        int MultiObjectInstance::getChannelCount() const
        {
            return mMixers.size();
        }
        
        
        int MultiObjectInstance::getInputChannelCount() const
        {
            if (mObjects.empty())
                return 0;
            return mObjects[0]->getInputChannelCount();
        }



        void MultiObjectInstance::connect(unsigned int channel, OutputPin& pin)
        {
            for (auto& object : mObjects)
                object->tryConnect(channel, pin);
        }
        
        
        void MultiObjectInstance::connect(MultiObjectInstance& inputMulti)
        {
            for (auto index = 0; index < getObjectCount(); ++index)
            {
                auto object = mObjects[index].get();
                auto inputObject = inputMulti.getObjectNonTyped(index % inputMulti.getObjectCount());
                object->connect(*inputObject);
            }
        }
        
        
        AudioObject* MultiObjectInstance::getObjectResource()
        {
            auto resource = getResource<MultiObject>();
            if (resource == nullptr)
                return nullptr;
            return resource->mObject.get();
        }



    }
    
}
