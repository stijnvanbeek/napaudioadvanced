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
         */
        class NAPAPI GraphObject : public AudioObject
        {
            RTTI_ENABLE(AudioObject)

        public:
            GraphObject() : AudioObject() { }

            /**
             * Pointer to the graph resource that this object uses.
             */
            ResourcePtr<Graph> mGraph;

        private:
            std::unique_ptr<AudioObjectInstance> createInstance() override;
        };


        /**
         * Instance of audio object that uses a graph to process it's output.
         */
        class NAPAPI GraphObjectInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)

        public:
            GraphObjectInstance(GraphObject& resource) : AudioObjectInstance(resource) { }

            // Initialize the component
            bool init(AudioService& audioService, utility::ErrorState& errorState) override;

            /**
             * Use this method to acquire an object within the graph by ID.
             * @return: a pointer to the object with the given ID.
             */
            template <typename T>
            T* getObject(const std::string& mID) { return mGraphInstance.getObject<T>(mID); }
            
            /**
             * Non typed version of @getObject().
             */
            AudioObjectInstance* getObjectNonTyped(const std::string& mID) { return mGraphInstance.getObjectNonTyped(mID); }

            OutputPin* getOutputForChannel(int channel) override;
            int getChannelCount() const override;
            InputPinBase* getInputForChannel(int channel) override;
            int getInputChannelCount() const override;

        private:
            GraphInstance mGraphInstance;
        };


    }

}
