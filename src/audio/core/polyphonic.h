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
            PolyphonicInstance() : AudioObjectInstance() { }
            PolyphonicInstance(const std::string& name) : AudioObjectInstance(name) { }

            // Inherited from AudioObjectInstance
            bool init(Voice& voice, int voiceCount, bool voiceStealing, int channelCount, NodeManager& nodeManager, utility::ErrorState& errorState);
            OutputPin* getOutputForChannel(int channel) override;
            int getChannelCount() const override;
            
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
             * @param endSegmtnt The index of the ending segment of the voice envelope's subsection that will be triggered.
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
