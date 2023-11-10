/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Std includes
#include <vector>

// Nap includes
#include <nap/resourceptr.h>
#include <rtti/object.h>
#include <rtti/factory.h>

// Audio includes
#include <audio/core/audioobject.h>


namespace nap
{
    
    namespace audio
    {
        
        using AudioObjectPtr = ResourcePtr<AudioObject>;
        
        
        /**
         * The Graph manages a number of different audio objects that are connected together to represent a DSP network to perform a specific task of mono or multichannel audio processing.
         * Internally it resolves links between the objects and manages order of initialization accordingly.
         * Note: this is the resource of the graph, it needs to be instantiated to an instance that can do the actual audio processing.
         */
        class NAPAPI Graph : public Resource
        {
            RTTI_ENABLE(Resource)
        public:
            Graph() = default;

            std::vector<AudioObjectPtr> mObjects;        ///< Property: 'Objects' The audio objects managed by the graph that are part of its DSP network.
            
            ResourcePtr<AudioObject> mOutput = nullptr;  ///< Property: 'Output' Pointer to an audio object in the graph that will be polled for output in order to present audio output.
            
            ResourcePtr<AudioObject> mInput = nullptr;   ///< Property: 'Input' Pointer to an effect object in the graph where audio input will be connected to.
        };
        
        
        /**
         * Instance of Graph that manages a number of different audio objects, connected together to represent a DSP network to perform a specific task of mono or multichannel audio processing.
         */
        class NAPAPI GraphInstance
        {
            RTTI_ENABLE()
            
        public:
            GraphInstance() = default;
            virtual ~GraphInstance() = default;

            // Delete copy and move constructors
            GraphInstance(const GraphInstance&) = delete;
            GraphInstance& operator=(const GraphInstance&) = delete;

            /**
             * Initializes the graph from its resource
             * @param resource The graph resource that the GraphInstance is instantiated from.
             * @param nodeManager The NodeManager this graph instance will be processed by.
             * @param errorState To log errors during the initialization process.
             * @return True on succesful initialization.
             */
            bool init(Graph& resource, audio::NodeManager& nodeManager, utility::ErrorState& errorState);
            
            /**
             * @return: an object within this graph by ID.
             */
            template <typename T>
            T* getObject(const std::string& name)
            {
                return rtti_cast<T>(getObjectNonTyped(name));
            }
            
            /**
             * Non typed version of getObject() for use in python.
             * @return raw pointer to object within the graph by ID name.
             */
            AudioObjectInstance* getObjectNonTyped(const std::string& name);
            
            /**
             * @return the output object of the graph as specified in the resource
             */
            AudioObjectInstance* getOutput() { return mOutput; }
            const AudioObjectInstance* getOutput() const { return mOutput; }

            /**
             * @return the input object of the graph as specified in the resource
             */
            AudioObjectInstance* getInput() { return mInput; }
            const AudioObjectInstance* getInput() const { return mInput; }
            
             /**
              * Adds an object to the graph at runtime. The graph takes over ownership.
              * @param object The audio object instance to add to the graph.
              */
            AudioObjectInstance& addObject(std::unique_ptr<AudioObjectInstance> object);
            
            /**
             * Adds an object to the graph and marks it to be the graph's input. The graph takes over ownership of the object.
              * @param object The audio object instance to add to the graph as input.
             */
            AudioObjectInstance& addInput(std::unique_ptr<AudioObjectInstance> object);
            
            /**
             * Adds an object to the graph and marks it to be the graph's output. The graph takes over ownership of the object.
              * @param object The audio object instance to add to the graph as output.
             */
            AudioObjectInstance& addOutput(std::unique_ptr<AudioObjectInstance> object);

        protected:
            /**
             * @return: all objects within the graph.
             */
            const std::vector<std::unique_ptr<AudioObjectInstance>>& getObjects() const { return mObjects; }
            
            /**
             * @return: the audio service that this graph runs on.
             */
            NodeManager& getNodeManager() { return *mNodeManager; }

        private:
            std::vector<std::unique_ptr<AudioObjectInstance>> mObjects;
            AudioObjectInstance* mOutput = nullptr;
            AudioObjectInstance* mInput = nullptr;
            NodeManager* mNodeManager = nullptr;
        };
        
        
    }
    
}
