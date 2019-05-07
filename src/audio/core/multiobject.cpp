#include "multiobject.h"

// Nap includes
#include <entity.h>
#include <nap/logger.h>

// Audio includes
#include <audio/core/audioobject.h>


// RTTI
RTTI_BEGIN_CLASS(nap::audio::MultiObject)
    RTTI_PROPERTY("Object", &nap::audio::MultiObject::mObject, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("InstanceCount", &nap::audio::MultiObject::mInstanceCount, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::MultiObjectInstance)
    RTTI_CONSTRUCTOR(nap::audio::MultiObject&)
    RTTI_FUNCTION("getObject", &nap::audio::MultiObjectInstance::getObject)
    RTTI_FUNCTION("getObjectCount", &nap::audio::MultiObjectInstance::getObjectCount)
    RTTI_FUNCTION("setActive", &nap::audio::MultiObjectInstance::setActive)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        

        std::unique_ptr<AudioObjectInstance> MultiObject::createInstance()
        {
            return std::make_unique<MultiObjectInstance>(*this);
        }
        
        
        bool MultiObjectInstance::init(AudioService& audioService, utility::ErrorState& errorState)
        {
            mAudioService = &audioService;
            
            MultiObject* resource = rtti_cast<MultiObject>(&getResource());
            for (auto i = 0; i < resource->mInstanceCount; ++i)
            {
                auto instance = resource->mObject->instantiate<AudioObjectInstance>(audioService, errorState);
                if (instance == nullptr)
                    return false;
                mObjects.emplace_back(std::move(instance));
            }
            
            // initialize the mixers
            if (mObjects.size() > 0)
            {
                auto channelCount = (*mObjects.begin())->getChannelCount();
                for (auto channel = 0; channel < channelCount; ++channel)
                {
                    // we dont connect anything yet
                    mMixers.emplace_back(mAudioService->makeSafe<MixNode>(mAudioService->getNodeManager()));
                }
            }
            return true;
        }

        
        AudioObjectInstance* MultiObjectInstance::getObject(unsigned int index)
        {
            if (index < mObjects.size())
                return mObjects[index].get();
            
            nap::Logger::warn("MultiObjectInstance %s: index for getObject() out of bounds", mID.c_str());
            return nullptr;
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



    }
    
}
