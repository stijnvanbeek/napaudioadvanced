#include "graphobject.h"

// Nap includes
#include <entity.h>

// Audio includes
#include <audio/core/audioobject.h>


// RTTI
RTTI_BEGIN_CLASS(nap::audio::GraphObject)
    RTTI_PROPERTY("Graph", &nap::audio::GraphObject::mGraph, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::audio::GraphObjectInstance)
    RTTI_FUNCTION("getObject", &nap::audio::GraphObjectInstance::getObjectNonTyped)
RTTI_END_CLASS


namespace nap
{

    namespace audio
    {

        std::unique_ptr<AudioObjectInstance> GraphObject::createInstance(AudioService& service, utility::ErrorState& errorState)
        {
            auto instance = std::make_unique<GraphObjectInstance>();
            if (!instance->init(*mGraph, service, errorState))
                return nullptr;
            
            return instance;
        }


        bool GraphObjectInstance::init(Graph& graph, AudioService& audioService, utility::ErrorState& errorState)
        {
            if (!mGraphInstance.init(graph, errorState))
            {
                errorState.fail("Fail to init graph.");
                return false;
            }
            
            return true;
        }
        
    }

}
