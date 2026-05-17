/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Nap includes
#include <component.h>

// Audio includes
#include <audio/core/audioobject.h>
#include <audio/core/graph.h>

namespace nap
{

    namespace audio
    {

        /**
         * Audio object that uses a graph to process it's output.
         * Basically an AudioObject wrapper for a Graph.
         */
        class NAPAPI GraphObject : public AudioObject
        {
            RTTI_ENABLE(AudioObject)

        public:
            GraphObject() : AudioObject() { }

            ResourcePtr<Graph> mGraph = nullptr; ///< Property: 'Graph' Pointer to the graph resource that this object uses.
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;
        };
        
        
        /**
         * Instance of audio object that uses a graph to process it's output.
         * Basically an AudioObjectInstance wrapper for a GraphInstance.
         */
        class NAPAPI GraphObjectInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)

        public:
            GraphObjectInstance() : AudioObjectInstance() { }
            GraphObjectInstance(const std::string& name) : AudioObjectInstance(name) { }

            /**
             * Instantiates and initializes this GraphObjectInstance from a Graph resource.
             * @param graph Resource of the Graph that this object will instantiate.
             * @param nodeManager The NodeManager that the graph will be processed by.
             * @param errorState Logs error messages if the initialization of the graph fails.
             * @return True on succesful initialization.
             */
            bool init(Graph& graph, NodeManager& nodeManager, utility::ErrorState& errorState);

            /**
             * Use this method to acquire an object within the graph by ID.
             * @return: a pointer to the object with the given ID.
             */
            template <typename T>
            T* getObject(const std::string& mID) { return mGraphInstance.getObject<T>(mID); }
            
            /**
             * Non typed version of @getObject() for python usage.
             * @return a raw pointer to the object with the given ID.
             */
            AudioObjectInstance* getObjectNonTyped(const std::string& mID) { return mGraphInstance.getObjectNonTyped(mID); }
            
            /**
             * Adds an object to the graph. The graph takes over ownership.
             * @param object Object to add to the graph.
             */
            AudioObjectInstance& addObject(std::unique_ptr<AudioObjectInstance> object) { return mGraphInstance.addObject(std::move(object)); }
            
            /**
             * Adds an object to the graph and marks it to be the graph's input. The graph takes over ownership of the object.
             * @param object Object to add to the graph as input.
             */
            AudioObjectInstance& addInput(std::unique_ptr<AudioObjectInstance> object)  { return mGraphInstance.addInput(std::move(object)); }
            
            /**
             * Adds an object to the graph and marks it to be the graph's output. The graph takes over ownership of the object.
             * @param object Object to add to the graph as output.
             */
            AudioObjectInstance& addOutput(std::unique_ptr<AudioObjectInstance> object) { return mGraphInstance.addOutput(std::move(object)); }


            // Multichannel implementation
            OutputPin* getOutputForChannel(int channel) override;
            int getChannelCount() const override;
            void connect(unsigned int channel, OutputPin& pin) override;
			void disconnect(unsigned int channel, OutputPin& pin) override;
            int getInputChannelCount() const override;
            
        private:
            GraphInstance mGraphInstance;
        };                

    }

}
