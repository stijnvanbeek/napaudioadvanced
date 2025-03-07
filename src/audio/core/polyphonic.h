/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Std includes
#include <mutex>

// Audio includes
#include <audio/utility/safeptr.h>
#include <audio/core/audioobject.h>
#include <audio/core/voice.h>
#include <audio/node/mixnode.h>

namespace nap
{
    
    namespace audio
    {
    
        class PolyphonicInstance;
        
        
        /**
         * This object manages a pool of voices for polyphonic playback.
         * The most common use would be musical instrument emulation.
         * For each "note" played on the isntrument one voice in the pool will be used.
         * When the playback of the note is finished the voice will be disconnected from the DSP processing graph.
         */
        class NAPAPI Polyphonic : public AudioObject
        {
            RTTI_ENABLE(AudioObject)

        public:
            Polyphonic() : AudioObject() { }

            ResourcePtr<Voice> mVoice;     ///< Property: 'Voices' This points to the voice graph resource defining the patch for a single voice in the polyphonic system.
            
            int mVoiceCount = 1;           ///< Property: 'VoiceCount' Number of voices in the voice pool. This indicates the maximum number of voices playing at the same time.
            
            bool mVoiceStealing = true;    ///< Property 'VoiceStealing' If set to true, every time the user tries to play more voices than there are present in the pool, the voice that has been playing for the longest time will be "stolen" and used to perform the new play command.
            
            int mChannelCount = 1;         ///< Property: 'ChannelCount' The number of channels that the object outputs. Beware that this dos not to be equal to the number of channels of the voice, as it is possible to play a voice on a specific set of output channels of the polyphonic object. See also @PolyphonicObjectInstance::playOnChannels().

            ResourcePtr<AudioObject> mInput; ///< Property: 'Input' This object from the same graph as the polyphonic will be connected to each of the voice's inputs.
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;
        };

        
        /**
         * Instance of object manages a pool of voices for polyphonic playback.
         */
        class NAPAPI PolyphonicInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)

        public:
            template <typename ObjectInstanceType>
            using ObjectMap = std::map<VoiceInstance*, ObjectInstanceType*>;

        public:
            PolyphonicInstance() : AudioObjectInstance() { }
            PolyphonicInstance(const std::string& name) : AudioObjectInstance(name) { }

            // Inherited from AudioObjectInstance
            bool init(Voice& voice, int voiceCount, bool voiceStealing, int channelCount, NodeManager& nodeManager, utility::ErrorState& errorState);
            OutputPin* getOutputForChannel(int channel) override;
            int getChannelCount() const override;
            void connect(unsigned int channel, OutputPin& pin) override;
			void disconnect(unsigned int channel, OutputPin& pin) override;
            int getInputChannelCount() const override;

            
            /**
             * @return Returns the first voice in the pool that is not being used (Voice::isBusy() == false) for playback.
             * Before a voice is returned by this method it will already be marked as busy.
             * Once the envelope of the voice has been played and finished the voice will be freed again.
             */
            VoiceInstance* findFreeVoice();
            
            /**
             * Starts playing a voice by calling it's play() method and connecting it's output to this object's mixer.
             * Before being passed to this method a voice has te be acquired and reserved for use using findFreeVoice().
             * @param voice The voice to be played.
             * @param duration The total duration of the envelope. This parameter will only have effect if the voice's envelope data has segments with a relative duration. See Envelope for more info.
             */
            void play(VoiceInstance* voice, TimeValue duration = 0);

            /**
             * Starts playing a voice by calling it's play() method and connecting it's output to this object's mixer.
             * Before being passed to this method a voice has te be acquired and reserved for use using findFreeVoice().
             * This method does not trigger the full voice envelope, but only a subsection of it.
             * @param voice The voice that the envelope section will be played on.
             * @param startSegment The index of the starting segment of the voice envelope's subsection that will be triggered.
             * @param endSegment The index of the ending segment of the voice envelope's subsection that will be triggered.
             * @param startValue As envelope segments only define their destination value, this parameter can be used to define the starting value of the first segment. Normally this will be 0 to avoid clicks and pops.
             * @param totalDuration The total duration of the envelope's subsection. This parameter will only have effect if the voice's envelope data has segments with a relative duration. See Envelope for more info.
             */
            void playSection(VoiceInstance* voice, int startSegment, int endSegment, ControllerValue startValue = 0, TimeValue totalDuration = 0);

            /**
             * Starts playing a voice by calling it's play() method and connecting it's output to this object's mixer.
             * Before being passed to this method a voice has te be acquired and reserved for use using findFreeVoice().
             * As opposed to @play() this method connects the voice's output to the specified @channels of this polyphic object's output mixer.
             * @param voice The voice to be played.
             * @param channels Vector containing the indices of the output channels of this polyphonic that the voice will be connected to. The number of elements in the vector has to equal the number of output channels of the voice.
             * @param duration The total duration of the envelope. This parameter will only have effect if the voice's envelope data has segments with a relative duration. See Envelope for more info.
             */
            void playOnChannels(VoiceInstance* voice, std::vector<unsigned int> channels, TimeValue duration = 0);

            /**
             * Starts playing a voice by calling it's play() method and connecting it's output to this object's mixer.
             * Before being passed to this method a voice has te be acquired and reserved for use using findFreeVoice().
             * This method does not trigger the full voice envelope, but only a subsection of it.
             * As opposed to @play() this method connects the voice's output to the specified @channels of this polyphic object's output mixer.
             * @param voice The voice that the envelope section will be played on.
             * @param channels Vector containing the indices of the output channels of this polyphonic that the voice will be connected to. The number of elements in the vector has to equal the number of output channels of the voice.
             * @param startSegment The index of the starting segment of the voice envelope's subsection that will be triggered.
             * @param endSegment The index of the ending segment of the voice envelope's subsection that will be triggered.
             * @param startValue As envelope segments only define their destination value, this parameter can be used to define the starting value of the first segment. Normally this will be 0 to avoid clicks and pops.
             * @param totalDuration The total duration of the envelope's subsection. This parameter will only have effect if the voice's envelope data has segments with a relative duration. See Envelope for more info.
             */
            void playSectionOnChannels(VoiceInstance* voice, std::vector<unsigned int> channels, int startSegment, int endSegment, ControllerValue startValue = 0, TimeValue totalDuration = 0);

            /**
             * Stops playing the voice by telling it to fade out it's envelope
             * Once the envelope is faded out this will trigger the voice to be disconnected from this object's output mixers
             * @param voice The voice to be played.
             * @param fadeOutTime The fadeout time in ms from the moment this method is called.
             */
            void stop(VoiceInstance* voice, TimeValue fadeOutTime);

            /**
             * Stops the polyphonic hard by disconnecting all its voices.
             * Only call this while the polyphonic is not being processed, otherwise it will result in clicks and pops.
             */
            void reset();
            
            /**
             * @return the number of voices that are currently playing.
             */
            int getBusyVoiceCount() const;

            /**
             * Fills a map with the AudioObjectInstances with a certain name for each voice.
             * This function can be used at init to gather the AudioObjectInstances from the voices that will be manipulated at runtime.
             * This makes sure the string lookup within the voices is done only once at init.
             * @tparam ObjectInstanceType The type of the AudioObjectInstance that will be looked up from the VoiceInstances.
             * @param name The name of the AudioObjectInstance within the Voice.
             * @param objectMap The map that will be filled with the AudioObjectInstances mapped to the corresponding VoiceInstance in which they live.
             * @param errorState If no AudioObjectInstance of ObjectInstanceType with this name was found within the voices, this is logged here.
             * @return  True on success
             */
            template <typename ObjectInstanceType>
            bool getObjectMap(const std::string& name, ObjectMap<ObjectInstanceType>& objectMap, utility::ErrorState& errorState)
            {
                for (auto& voice : mVoices)
                {
                    ObjectInstanceType* object = voice->getObject<ObjectInstanceType>(name);
                    if (object == nullptr)
                    {
                        errorState.fail("No object %s with corresponding type found in polyphonic.", name.c_str());
                        objectMap.clear();
                        return false;
                    }
                    objectMap[voice.get()] = object;
                }
                return true;
            }
            
        private:
            void connectVoice(VoiceInstance* voice);

            Slot<VoiceInstance&> voiceFinishedSlot = { this, &PolyphonicInstance::voiceFinished };
            void voiceFinished(VoiceInstance& voice);
            
            std::vector<std::unique_ptr<VoiceInstance>> mVoices;
            std::vector<SafeOwner<MixNode>> mMixNodes;
            
            NodeManager* mNodeManager = nullptr;
            bool mVoiceStealing = true;
        };
        
    }
    
}
