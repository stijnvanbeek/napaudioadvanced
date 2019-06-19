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
         */
        class NAPAPI Graph : public Resource
        {
            RTTI_ENABLE(Resource)
        public:
            Graph(AudioService& service) : mAudioService(service)  { }

            /**
             * The audio objects managed by the graph that are part of it's DSP network.
             */
            std::vector<AudioObjectPtr> mObjects;
            
            /**
             * Pointer to an audio object in the graph that will be polled for output in order to present audio output.
             */
            ResourcePtr<AudioObject> mOutput = nullptr;
            
            /**
             * Pointer to an effect object in the graph where audio input will be connected to.
             */
            ResourcePtr<AudioObject> mInput = nullptr;
            
            /**
             * Returns the audio service that instances of this graph will perform their DSP processing on.
             */
            AudioService& getAudioService() { return mAudioService; }
            
        private:
            AudioService& mAudioService;
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
            
            bool init(Graph& resource, utility::ErrorState& errorState);
            
            Graph& getResource() { return *mResource; }

            /**
             * @return: an object within this graph by ID.
             */
            template <typename T>
            T* getObject(const std::string& mID)
            {
                return rtti_cast<T>(getObjectNonTyped(mID));
            }
            
            /**
             * Non typed version of getObject() for use in python.
             */
            AudioObjectInstance* getObjectNonTyped(const std::string& mID);
            
            /**
             * Returns the output object of the graph as specified in the resource
             */
            AudioObjectInstance& getOutput() { return *mOutput; }
            const AudioObjectInstance& getOutput() const { return *mOutput; }

            /**
             * Returns the input object of the graph as specified in the resource
             */
            AudioObjectInstance* getInput() { return mInput; }
            const AudioObjectInstance* getInput() const { return mOutput; }

        protected:
            /**
             * @return: all objects within the graph.
             */
            const std::vector<std::unique_ptr<AudioObjectInstance>>& getObjects() const { return mObjects; }

        private:
            std::vector<std::unique_ptr<AudioObjectInstance>> mObjects;
            AudioObjectInstance* mOutput = nullptr;
            AudioObjectInstance* mInput = nullptr;
            Graph* mResource = nullptr;
        };
        
        
        using GraphObjectCreator = rtti::ObjectCreator<Graph, AudioService>;
        
    }
    
}
