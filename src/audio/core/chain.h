/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
         * A chain of AudioObjects whose inputs and outputs are connected serially.
         * A chain can never be empty.
         */
        class NAPAPI Chain : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
            
        public:
            Chain() : AudioObject() { }
            
            std::vector<ResourcePtr<AudioObject>> mObjects;    ///< Property: 'Objects' Resources of objects in the chain
            ResourcePtr<AudioObject> mInput = nullptr;         ///< Property: 'Input' Pointer to the object that will be connected to the input of the chain

        private:
            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;
        };
        

        /**
         * Instance of Chain
         */
        class NAPAPI ChainInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
            
        public:
            // Inherited from AudioObjectInstance
            ChainInstance() : AudioObjectInstance() { }
            ChainInstance(const std::string& name) : AudioObjectInstance(name) { }

            OutputPin* getOutputForChannel(int channel) override { return mObjects.back()->getOutputForChannel(channel); }
            int getChannelCount() const override { return mObjects.back()->getChannelCount(); }
            void connect(unsigned int channel, OutputPin& pin) override { mObjects[0]->connect(channel, pin); }
			void disconnect(unsigned int channel, OutputPin& pin) override { mObjects[0]->disconnect(channel, pin); }
            int getInputChannelCount() const override { return mObjects[0]->getInputChannelCount(); }
            
            /**
             * Use this method to acquire an object within the chain by index.
             * @param index Index of the requested object in the chain.
             * @return: a pointer to the object with the given index.
             */
            template <typename T>
            T* getObject(unsigned int index) { return rtti_cast<T>(mObjects[index].get()); }

            /**
             * @return The first object in the chain.
             */
            template <typename T>
            T* front() { return rtti_cast<T>(mObjects[0].get()); }

            /**
             * @return The last object in the chain.
             */
            template <typename T>
            T* back() { return rtti_cast<T>(mObjects.back().get()); }

            /**
             * Use this method to acquire an object within the chain by index.
             * @param index Index of the requested object in the chain.
             * @return: a pointer to the object with the given index.
             */
            template <typename T>
            const T* getObject(unsigned int index) const { return rtti_cast<T>(mObjects[index].get()); }

            /**
             * @return The first object in the chain.
             */
            template <typename T>
            const T* front() const { return rtti_cast<T>(mObjects[0].get()); }

            /**
             * @return The last object in the chain.
             */
            template <typename T>
            const T* back() const { return rtti_cast<T>(mObjects.back().get()); }

            /**
             * Same as getObject() but returns a non typed baseclass.
             * @param index Index of the requested object in the chain.
             * @return a pointer to the object with the given index.
             */
            AudioObjectInstance* getObjectNonTyped(unsigned int index);

            /**
             * Instantiates a new audio object and adds it to the back of the chain.
             * @param resource Resource from which the new object will be instantiated
             * @param nodeManager NodeManager on which the new object will be processed.
             * @param errorState
             * @return
             */
            bool addObject(AudioObject& resource, NodeManager& nodeManager, utility::ErrorState& errorState);
            bool addObject(std::unique_ptr<AudioObjectInstance> object, utility::ErrorState& errorState);

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
            virtual std::unique_ptr<AudioObjectInstance> instantiateObjectInChain(AudioObject& resource, NodeManager& audioService, utility::ErrorState& errorState);
            
            /**
             * Overwrite this method to manipulate the way two objects in the chain are connected.
             */
            virtual bool connectObjectsInChain(AudioObjectInstance& source, AudioObjectInstance& destination, utility::ErrorState& errorState);
        };
        
        
        
    }
    
}
