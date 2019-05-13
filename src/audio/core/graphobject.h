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
            ResourcePtr<Graph> mGraph = nullptr;
            
            /**
             * Pointer to an audio object in the graph that will be polled for output in order to present this object's audio output.
             */
            ResourcePtr<AudioObject> mOutput = nullptr;

        private:
            std::unique_ptr<AudioObjectInstance> createInstance() override;
        };
        
        
        class NAPAPI GraphEffect : public GraphObject
        {
            RTTI_ENABLE(GraphObject)
            
        public:
            GraphEffect() : GraphObject() { }
            
            /**
             * Pointer to an effect object in the graph where audio input will be connected to.
             */
            ResourcePtr<AudioObject> mInput = nullptr;
            
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

            OutputPin* getOutputForChannel(int channel) override { return mOutput->getOutputForChannel(channel); }
            int getChannelCount() const override { return mOutput->getChannelCount(); }

        private:
            GraphInstance mGraphInstance;
            AudioObjectInstance* mOutput = nullptr;
        };
        
        
        /**
         * Instance of audio effect that uses a graph to perform its processing.
         */
        class NAPAPI GraphEffectInstance : public GraphObjectInstance, public IMultiChannelInput
        {
            RTTI_ENABLE(AudioObjectInstance)
            
        public:
            GraphEffectInstance(GraphEffect& resource) : GraphObjectInstance(resource) { }
            
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
            
            OutputPin* getOutputForChannel(int channel) override { return mOutput->getOutputForChannel(channel); }
            int getChannelCount() const override { return mOutput->getChannelCount(); }
            
            void connect(unsigned int channel, OutputPin& pin) override { mInput->connect(channel, pin); }
            int getInputChannelCount() const override { return mInput->getInputChannelCount(); }
            
        private:
            GraphInstance mGraphInstance;
            AudioObjectInstance* mOutput = nullptr;
            IMultiChannelInput* mInput = nullptr;
        };
    }

}
