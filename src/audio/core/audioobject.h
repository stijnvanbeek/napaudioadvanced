#pragma once

// Nap includes
#include <rtti/factory.h>
#include <nap/resource.h>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/core/multichannel.h>
#include <audio/utility/safeptr.h>
#include <audio/service/audioservice.h>

namespace nap
{
    
    namespace audio
    {
        
        // Forward delcarations
        class AudioObject;
        
        
        /**
         * Instance of a object that generates audio output for one or more channels
         */
        class NAPAPI AudioObjectInstance : public rtti::Object, public IMultiChannel
        {
            RTTI_ENABLE()
            
        public:
            AudioObjectInstance(AudioObject& resource) : mResource(resource) { }
            
            /**
             * This method has to be overwritten by all descendants to initialize the instance.
             * Normally it will create all the Nodes owned by this instance and connect them.
             */
            virtual bool init(AudioService& service, utility::ErrorState& errorState) = 0;
            
            /**
             * @return: the resource this instance is created from.
             */
            AudioObject& getResource() { return mResource; }
            
            /**
             * @return: the resource this instance is created from cast to type T.
             */
            template <typename T>
            T* getResource() { return rtti_cast<T>(&mResource); }
            
        private:
            AudioObject& mResource;
        };
        
        
        
        /**
         * AudioObject is a base class for objects that generate single- or multichannel audio using one or more Nodes in a DSP system.
         * AudioObjects can be linked together to build a more complex DSP system in a Graph.
         * AudioObject is a resource that can be instantiated.
         */
        class NAPAPI AudioObject : public Resource
        {
            RTTI_ENABLE(Resource)
            
        public:
            AudioObject() = default;
            
            /**
             * This method can be used during the initialization of a Graph of AudioObjects to acquire a pointer to the instance of this object in the graph.
             */
            AudioObjectInstance* getInstance() { return mInstance; }
            
            /**
             * This method spawns an instance of this resource.
             */
            template <typename T>
            std::unique_ptr<T> instantiate(AudioService& service, utility::ErrorState& errorState);
            
        private:
            /**
             * This methods need to be overwritten by all descendants to return an instance of this resource.
             */
            virtual std::unique_ptr<AudioObjectInstance> createInstance() { return nullptr; }
            
            AudioObjectInstance* mInstance = nullptr;
        };
        
        
        template <typename T>
        std::unique_ptr<T> AudioObject::instantiate(AudioService& service, utility::ErrorState& errorState)
        {
            auto instance = createInstance();
            mInstance = instance.release();
            if (errorState.check(mInstance->init(service, errorState), "Failed to instantiate object %s", mID.c_str()))
                return std::unique_ptr<T>(rtti_cast<T>(mInstance));
            else
                return nullptr;
        }
        

    }
        
}


