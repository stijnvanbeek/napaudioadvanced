#pragma once

// Nap includes
#include <component.h>
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/audioobject.h>
#include <audio/node/mixnode.h>

namespace nap
{
    
    namespace audio
    {
    

        /**
         * Audio object that owns multiple instances of an audio object and mixes their output
         */
        class NAPAPI MultiObject : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
            
        public:
            MultiObject() : AudioObject() { }
            
            /**
             * Pointer to the audio object resource that this object uses.
             */
            ResourcePtr<AudioObject> mObject;
            int mInstanceCount = 1; ///< Property: 'InstanceCount' Number of instances of the object that will be created on initialization.
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance() override;
        };
        
        
        /**
         * Instance of audio object that internally holds multiple objects of one type and mixes their output.
         */
        class NAPAPI MultiObjectInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
            
        public:
            MultiObjectInstance(MultiObject& resource) : AudioObjectInstance(resource) { }
            
            // Initialize the object
            bool init(AudioService& audioService, utility::ErrorState& errorState) override;
            
            /**
             * Use this method to acquire one of the managed objects.
             * @return: a pointer to the object with the given index.
             */
            AudioObjectInstance* getObject(unsigned int index);
            
            /**
             * @return: The number of managed objects.
             */
            int getObjectCount() const { return mObjects.size(); }
            
            /**
             * Use this method to activate one of the managed objects by connecting it to the output mixer.
             * Returns true on success, false if @object has not been found.
             */
            bool setActive(AudioObjectInstance* object, bool isActive);
            
        private:
            OutputPin* getOutputForChannel(int channel) override;
            int getChannelCount() const override;
            
            std::vector<std::unique_ptr<AudioObjectInstance>> mObjects;
            std::vector<SafeOwner<MixNode>> mMixers;
            
            AudioService* mAudioService = nullptr;
        };
        
    }
    
}
