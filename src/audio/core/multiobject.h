/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
         * Audio object that owns multiple instances of an audio object and mixes their outputs.
         */
        class NAPAPI MultiObject : public AudioObject
        {
            RTTI_ENABLE(AudioObject)

        public:
            MultiObject() : AudioObject() { }

            int mInstanceCount = 1;                     ///< Property: 'InstanceCount' Number of instances of the object that will be created on initialization.
            bool mIsActive = true;                      ///< Property: 'IsActive' Indicates wether the objects within the MultiObject are active at initialization. Active means: connected to the output mixers.
            
            ResourcePtr<AudioObject> mObject = nullptr; ///< Property: 'Object' Pointer to the audio object resource that this object uses.
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;
        };
        
        
        /**
         * Instance of audio object that internally holds multiple objects of one type and mixes their outputs.
         * By default all objects are not connected to the mixers. Use @setActive() to do so.
         */
        class NAPAPI MultiObjectInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
            
        public:
            MultiObjectInstance() : AudioObjectInstance() { }
            MultiObjectInstance(const std::string& name) : AudioObjectInstance(name) { }

            /**
             * Initializes the object
             * @param objectResource Resource of the AudioObject of which this MultiObjectInstance will instantiate and manage multiple instances.
             * @param instanceCount Number of instances that the MultiObjectInstance will spawn from the objectResource.
             * @param isActive If set to true the instances of the managed objects will be connected to the output mixers after they are spawned.
             * @param nodeManager The NodeManager that the MultiObjectInstance is processed on.
             * @param errorState Logs errors during the initialization process.
             * @return True if the initialization is succesful.
             */
            bool init(AudioObject& objectResource, int instanceCount, bool isActive, NodeManager& nodeManager, utility::ErrorState& errorState);
            
             /**
              * Use this method to acquire one of the managed objects.
              * @tparam T Instance type of the managed object.
              * @param index Index of the requested object
              * @return A pointer to the object with the given index. Nullptr if type T does not match the managed object's instance type.
              */
            template <typename T>
            T* getObject(unsigned int index) { return rtti_cast<T>(mObjects[index].get()); }

            /**
             * @tparam T Instance type of the managed object.
             * @return Pointer to the first managed object in the list. Nullptr if type T does not match the managed object's instance type.
             */
            template <typename T>
            T* front() { return rtti_cast<T>(mObjects[0].get()); }

            /**
             * @tparam T Instance type of the managed object.
             * @return Pointer to the last managed object in the list. Nullptr if type T does not match the managed object's instance type.
             */
            template <typename T>
            T* back() { return rtti_cast<T>(mObjects.back().get()); }

            /**
             * Use this method to acquire one of the managed objects.
             * @tparam T Instance type of the managed object.
             * @param index Index of the requested object
             * @return A pointer to the object with the given index. Nullptr if type T does not match the managed object's instance type.
             */
            template <typename T>
            const T* getObject(unsigned int index) const { return rtti_cast<T>(mObjects[index].get()); }

            /**
             * @tparam T Instance type of the managed object.
             * @return Pointer to the first managed object in the list. Nullptr if type T does not match the managed object's instance type.
             */
            template <typename T>
            const T* front() const { return rtti_cast<T>(mObjects[0].get()); }

            /**
             * @tparam T Instance type of the managed object.
             * @return Pointer to the last managed object in the list. Nullptr if type T does not match the managed object's instance type.
             */
            template <typename T>
            const T* back() const { return rtti_cast<T>(mObjects.back().get()); }

            /**
             * Non typed version of getObject() for the python binding.
             * @param index Index of the requested object
             * @return A pointer to the object with the given index. Nullptr if index is out of bounds.
             */
            AudioObjectInstance* getObjectNonTyped(unsigned int index);

            /**
             * @return: The number of managed objects.
             */
            int getObjectCount() const { return mObjects.size(); }
            
            /**
             * Creates, initializes and adds a new object to the multi object at runtime and returns it.
             * Be aware that after adding a new object nothing is connected to it yet and it will be inactive.
             * @tparam T Type of the managed object's instance.
             * @param errorState Logs errors during the initialization of the new object.
             * @return Pointer to the new object, nullptr if initialization of the new object fails or T does not match the managed object's instance type.
             */
            template <typename T>
            T* addObject(utility::ErrorState& errorState) { return rtti_cast<T>(addObjectNonTyped(errorState)); }

            /**
             * Creates, initializes and adds a new object to the multi object at runtime and returns it.
             * Be aware that after adding a new object nothing is connected to it yet and it will be inactive.
             * @param errorState Logs errors during the initialization of the new object.
             * @return Pointer to the new object, nullptr if initialization of the new object fails.
             */
            AudioObjectInstance* addObjectNonTyped(utility::ErrorState& errorState);
            
            /**
             * Use this method to activate or deactivate one of the managed objects by connecting or disconnecting it to/from the output mixer.
             * @return true on success, false if @object has not been found.
             */
            bool setActive(AudioObjectInstance* object, bool isActive);
            
            /**
             * @return the mix of a certain channel of all objects.
             */
            OutputPin* getOutputForChannel(int channel) override;
            
            /**
             * @return the number of output channels per object and of the MultiObjectInstance as a whole.
             */
            int getChannelCount() const override;
            
            /**
             * Tries to connect &pin to @channel for each object in the MultiObject.
             * @param channel The channel within the managed object to connect the pin to.
             * @param pin The pin to connect to each of the managed objects.
             */
            void connect(unsigned int channel, OutputPin& pin) override;

			/**
			 * Tries to disconnect &pin from @channel for each object in the MultiObject.
			 * @param channel The channel within the managed object to disconnect the pin from.
			 * @param pin The pin to connect to each of the managed objects.
			 */
			void disconnect(unsigned int channel, OutputPin& pin) override;

			/**
             * @return the number of input channels of each object in the MultiObject.
             */
            int getInputChannelCount() const override;
            
            /**
             * Connects the outputs of all objects of another MultiObject to the inputs of all this MultiEffect's objects.
             * This method can be used to connect two MultiObjectInstances by connecting their managed objects and ignoring the output mixers of the input MultiObject.
             * @param multi The MultiObjectInstances whose managed objects will be connected to this object's managed objects.
             */
            void connect(MultiObjectInstance& multi);
            
        protected:
            std::vector<std::unique_ptr<AudioObjectInstance>> mObjects;
            
        private:
            std::vector<SafeOwner<MixNode>> mMixers;            
            NodeManager* mNodeManager = nullptr;
            AudioObject* mObjectResource = nullptr;
        };
        

    }
    
}
