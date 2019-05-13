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
            template <typename T>
            T* getObject(unsigned int index) { return rtti_cast<T>(getObjectNonTyped(index)); }
            
            AudioObjectInstance* getObjectNonTyped(unsigned int index);
            
            /**
             * @return: The number of managed objects.
             */
            int getObjectCount() const { return mObjects.size(); }
            
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
            
        protected:
            std::vector<std::unique_ptr<AudioObjectInstance>> mObjects;
            
        private:
            std::vector<SafeOwner<MixNode>> mMixers;            
            AudioService* mAudioService = nullptr;
        };
        
        
        class NAPAPI MultiEffect : public MultiObject
        {
            RTTI_ENABLE(MultiObject)
            
        public:
            MultiEffect() : MultiObject() { }
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance() override;
        };
        
        
        class NAPAPI MultiEffectInstance : public MultiObjectInstance, public IMultiChannelInput
        {
            RTTI_ENABLE(MultiObjectInstance)
        
        public:
            MultiEffectInstance(MultiEffect& resource) : MultiObjectInstance(resource) { }

            // Inherited from IMultiChannelInput
            int getInputChannelCount() const override;
            void connect(unsigned int channel, OutputPin& pin) override;
            
            /**
             * Connects the outputs of all objects of another MultiObject to the inputs of all this MultiEffect's objects.
             */
            void connect(MultiObjectInstance& multi);
        };
        
    }
    
}
