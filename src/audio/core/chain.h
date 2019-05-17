#pragma once

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/audioobject.h>
#include <audio/core/multiobject.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * A chain of AudioObjects that whose inputs and outputs are connected serially.
         * A chain can never be empty.
         */
        class NAPAPI Chain : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
        public:
            Chain() : AudioObject() { }
            
            std::vector<ResourcePtr<AudioObject>> mObjects;
            ResourcePtr<AudioObject> mInput = nullptr;
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance() override;
        };
        
        
        class NAPAPI ChainInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
            
        public:
            ChainInstance(Chain& resource) : AudioObjectInstance(resource) { }
            bool init(AudioService& audioService, utility::ErrorState& errorState) override;
            
            OutputPin* getOutputForChannel(int channel) override { return mObjects.back()->getOutputForChannel(channel); }
            int getChannelCount() const override { return mObjects.back()->getChannelCount(); }
            void connect(unsigned int channel, OutputPin& pin) override { mObjects[0]->connect(channel, pin); }
            int getInputChannelCount() const override { return mObjects[0]->getInputChannelCount(); }
            
            /**
             * Use this method to acquire an object within the chain by index.
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
             * Returns the number of object
             */
            int getObjectCount() const { return mObjects.size(); }

        protected:
            std::vector<std::unique_ptr<AudioObjectInstance>> mObjects;
            
        private:
            /**
             * Overwrite this method to manipulate instantiation of objects in the chain.
             */
            virtual std::unique_ptr<AudioObjectInstance> instantiateObjectInChain(AudioObject& resource, AudioService& audioService, utility::ErrorState& errorState);
            
            /**
             * Overwrite this method to manipulate the way two objects in the chain are connected.
             */
            virtual bool connectObjectsInChain(AudioObjectInstance& source, AudioObjectInstance& destination, utility::ErrorState& errorState);
        };
        
        
        
        /**
         * Baseclass for MultiChain that serves the sole purpose of creating MultiChain descendants without properties.
         */
        class NAPAPI MultiChainBase : public Chain
        {
            RTTI_ENABLE(Chain)
        public:
            MultiChainBase() : Chain() { }
            
            int mChainCount = 1; ///< Property: 'ChainCount' Number of chains after initialization. This will overwrite the InstanceCount property in the MultiObjects within the Objects property.
            bool mIsActive; ///< Property: 'IsActive' Indicates wether the chains within the MultiChain are active at initialization. Active means: connected to the output mixers. This will overwrite the IsActive property in the MultiObjects within the Objects property.
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance() override;
        };
        
        
        
        /**
         * An effect chain that internally has to consist of MultiObjects.
         * All the output channels of all objects in the multiobjects will be connected to each other.
         * The outputs of the objects within last MultiObject in the chain are mixed and are the output of this chain.
         * In a lot of situations it would make more sense to use a @MultiObject consisting of Chains.
         * The MultiChain however is useful when we truly need to put MultiObjects that are treated as separate AudioObject within a chain.
         */
        class NAPAPI MultiChain : public MultiChainBase
        {
            RTTI_ENABLE(MultiChainBase)
        
        public:
            MultiChain() : MultiChainBase() { }
        };
        
        
        class NAPAPI MultiChainInstance : public ChainInstance
        {
            RTTI_ENABLE(ChainInstance)
        public:
            MultiChainInstance(MultiChainBase& resource) : ChainInstance(resource) { }
            
            /**
             * Adds a new object to all the multi objects in the chain.
             * Objects will be added to the back of the MultiObjects.
             * New objects will be automatically connected to each other but the chain will be inactive.
             */
            bool addChain(utility::ErrorState& errorState);
            int getChainCount() const { return front<MultiObjectInstance>()->getObjectCount(); }
            void setChainActive(int index, bool isActive);
            
        protected:
            std::unique_ptr<AudioObjectInstance> instantiateObjectInChain(AudioObject& resource, AudioService& audioService, utility::ErrorState& errorState) override;
            
        private:
            bool connectObjectsInChain(AudioObjectInstance& source, AudioObjectInstance& destination, utility::ErrorState& errorState) override;
        };
        
    }
    
}
