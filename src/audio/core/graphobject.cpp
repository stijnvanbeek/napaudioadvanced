#include "graphobject.h"

// Nap includes
#include <entity.h>

// Audio includes
#include <audio/core/audioobject.h>


// RTTI
RTTI_BEGIN_CLASS(nap::audio::GraphObject)
    RTTI_PROPERTY("Graph", &nap::audio::GraphObject::mGraph, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::GraphObjectInstance)
    RTTI_CONSTRUCTOR(nap::audio::GraphObject&)
    RTTI_FUNCTION("getObject", &nap::audio::GraphObjectInstance::getObjectNonTyped)
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
            
            return true;
        }
        
    }

}
