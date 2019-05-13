#include "graphobject.h"

// Nap includes
#include <entity.h>

// Audio includes
#include <audio/core/audioobject.h>


// RTTI
RTTI_BEGIN_CLASS(nap::audio::GraphObject)
    RTTI_PROPERTY("Graph", &nap::audio::GraphObject::mGraph, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Output", &nap::audio::GraphObject::mOutput, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::GraphObjectInstance)
    RTTI_CONSTRUCTOR(nap::audio::GraphObject&)
    RTTI_FUNCTION("getObject", &nap::audio::GraphObjectInstance::getObjectNonTyped)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::audio::GraphEffect)
    RTTI_PROPERTY("Input", &nap::audio::GraphEffect::mInput, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::GraphEffectInstance)
    RTTI_CONSTRUCTOR(nap::audio::GraphEffect&)
    RTTI_FUNCTION("getInputChannelCount", &nap::audio::GraphEffectInstance::getInputChannelCount)
    RTTI_FUNCTION("connect", &nap::audio::GraphEffectInstance::tryConnect)
RTTI_END_CLASS

namespace nap
{

    namespace audio
    {

        std::unique_ptr<AudioObjectInstance> GraphObject::createInstance()
        {
            return std::make_unique<GraphObjectInstance>(*this);
        }


        bool GraphObjectInstance::init(AudioService& audioService, utility::ErrorState& errorState)
        {
            GraphObject* resource = rtti_cast<GraphObject>(&getResource());
            if (!mGraphInstance.init(*resource->mGraph, errorState))
            {
                errorState.fail("Fail to init graph.");
                return false;
            }
            
            mOutput = mGraphInstance.getObject<AudioObjectInstance>(resource->mOutput->mID);
            if (mOutput == nullptr)
            {
                errorState.fail("Output not found: %s", resource->mOutput->mID.c_str());
                return false;
            }
            
            return true;
        }


        std::unique_ptr<AudioObjectInstance> GraphEffect::createInstance()
        {
            return std::make_unique<GraphEffectInstance>(*this);
        }
        
        
        bool GraphEffectInstance::init(AudioService& audioService, utility::ErrorState& errorState)
        {
            GraphEffect* resource = rtti_cast<GraphEffect>(&getResource());
            if (!mGraphInstance.init(*resource->mGraph, errorState))
            {
                errorState.fail("Fail to init graph.");
                return false;
            }
            
            mOutput = mGraphInstance.getObject<AudioObjectInstance>(resource->mOutput->mID);
            if (mOutput == nullptr)
            {
                errorState.fail("Output not found: %s", resource->mOutput->mID.c_str());
                return false;
            }
            
            auto inputObject = mGraphInstance.getObject<AudioObjectInstance>(resource->mInput->mID);
            if (inputObject == nullptr)
            {
                errorState.fail("Input not found: %s", resource->mInput->mID.c_str());
                return false;
            }
            
            mInput = dynamic_cast<IMultiChannelInput*>(inputObject);
            if (mInput == nullptr)
            {
                errorState.fail("Provided input has no input interface: %s", resource->mInput->mID.c_str());
                return false;
            }
            
            return true;
        }
        
        
    }

}
