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
         * Baseclass for MultiObject that serves the sole purpose of creating MultiObject descendants without properties.
         */
        class NAPAPI MultiObjectBase : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
            
        public:
            MultiObjectBase() : AudioObject() { }
            
            int mInstanceCount = 1; ///< Property: 'InstanceCount' Number of instances of the object that will be created on initialization.
            bool mIsActive = true; ///< Property: 'IsActive' Indicates wether the objects within the MultiObject are active at initialization. Active means: connected to the output mixers.
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance() override;
        };
    

        /**
         * Audio object that owns multiple instances of an audio object and mixes their output
         */
        class NAPAPI MultiObject : public MultiObjectBase
        {
            RTTI_ENABLE(MultiObjectBase)

        public:
            MultiObject() : MultiObjectBase() { }

            /**
             * Pointer to the audio object resource that this object uses.
             */
            ResourcePtr<AudioObject> mObject = nullptr;
        };
        
        
        /**
         * Instance of audio object that internally holds multiple objects of one type and mixes their output.
         * By default all objects are not connected to the mixers. Use @setActive() to do so.
         */
        class NAPAPI MultiObjectInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
            
        public:
            MultiObjectInstance(MultiObjectBase& resource) : AudioObjectInstance(resource) { }
            
            // Initialize the object
            bool init(AudioService& audioService, utility::ErrorState& errorState) override;
            
            /**
             * Use this method to acquire one of the managed objects.
             * @return: a pointer to the object with the given index.
             */
            template <typename T>
            T* getObject(unsigned int index) { return rtti_cast<T>(mObjects[index].get()); }
            template <typename T>
            T* front() { return rtti_cast<T>(mObjects[0].get()); }
            template <typename T>
            T* back() { return rtti_cast<T>(mObjects.back().get()); }
            
            template <typename T>
            const T* getObject(unsigned int index) const { return rtti_cast<T>(mObjects[index].get()); }
            template <typename T>
            const T* front() const { return rtti_cast<T>(mObjects[0].get()); }
            template <typename T>
            const T* back() const { return rtti_cast<T>(mObjects.back().get()); }
            
            AudioObjectInstance* getObjectNonTyped(unsigned int index);

            /**
             * @return: The number of managed objects.
             */
            int getObjectCount() const { return mObjects.size(); }
            
            /**
             * Adds a new object to the multi object and returns it.
             * Be aware that after adding a new object nothing is connected to it yet and it will be inactive.
             */
            template <typename T>
            T* addObject(utility::ErrorState& errorState) { return rtti_cast<T>(addObjectNonTyped(errorState)); }
            
            AudioObjectInstance* addObjectNonTyped(utility::ErrorState& errorState);
            
            /**
             * Use this method to activate one of the managed objects by connecting it to the output mixer.
             * Returns true on success, false if @object has not been found.
             */
            bool setActive(AudioObjectInstance* object, bool isActive);
            
            /**
             * Returns the mix of a certain channel of all objects.
             */
            OutputPin* getOutputForChannel(int channel) override;
            
            /**
             * Returns the number of channels per object.
             */
            int getChannelCount() const override;
            
            /**
             * Tries to connect &pin to @channel for each object in the MultiObject.
             */
            void connect(unsigned int channel, OutputPin& pin) override;
            
            /**
             * Returns the number of channels of each object in the MultiObject.
             */
            int getInputChannelCount() const override;
            
            /**
             * Connects the outputs of all objects of another MultiObject to the inputs of all this MultiEffect's objects.
             */
            void connect(MultiObjectInstance& multi);
            
        protected:
            virtual AudioObject* getObjectResource();
            
        protected:
            std::vector<std::unique_ptr<AudioObjectInstance>> mObjects;
            
        private:
            std::vector<SafeOwner<MixNode>> mMixers;            
            AudioService* mAudioService = nullptr;
        };
        

    }
    
}
