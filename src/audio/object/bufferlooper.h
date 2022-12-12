/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <audio/object/bufferplayer.h>
#include <audio/object/multiply.h>
#include <audio/core/polyphonic.h>

namespace nap
{
    
    namespace audio
    {
        
        // Forward declarations
        class BufferLooper;
        class BufferLooperInstance;
        

        /**
         * AudioObject that loops a section of a buffer by crossfading from the end of the section back to the start of the section.
         * It also allows the indicate the start offset of playback within the buffer and to transpose playback.
         * This object can combined with an ADSR envelope serve as the basis of a traditional sampler.
         */
        class NAPAPI BufferLooper : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
            
        public:
            /**
             * This class keeps the settings for a playback instruction of the BufferLooper
             */
            class NAPAPI Settings
            {
            public:
                bool init(utility::ErrorState& errorState);
                
                ResourcePtr<AudioBufferResource> mBufferResource = nullptr; ///< Property: 'Buffer' Pointer to the AudioBufferResource that contains the audio data to play back. Mostly an AudioFileResource.
                TimeValue mCrossFadeTime  = 1000.f;                         ///< Property: 'CrossFadeTime' Time in ms for the crossfade from the end of the loop to the start od the loop.
                TimeValue mStart = 0.f;                                     ///< Property: 'Start' Offset in ms where to start playback.
                TimeValue mLoopStart = 0.f;                                 ///< Property: 'LoopStart' Offset in ms where the looped section starts. Has to be greater than mStart.
                TimeValue mLoopEnd = 0.f;                                   ///< Property: 'LoopEnd' Offset in ms where the loop ends.
                ControllerValue mTranspose = 0.f;                           ///< Property: 'Transpose' in semitones This property is used to alter the playback speed.
                bool mLoop = true;                                          ///< Property: 'Loop' True if playback should loop. When this is false mLoopStart and mLoopEnd will be ignored.

                /**
                 * @return The length of the looped section minus the crossfades at the start end the end.
                 */
                TimeValue getLoopSustainDuration() const { return mLoopSustainDuration; }

                /**
                 * @return The length of the section between the start of playback and the start of the loop.
                 */
                TimeValue getFirstSustainDuration() const { return mFirstSustainDuration; }
                
            private:
                TimeValue mLoopSustainDuration = 0.f;
                TimeValue mFirstSustainDuration = 0.f;
            };
            
        public:
            BufferLooper() : AudioObject() { }
            
            bool init(utility::ErrorState& errorState) override;
            
            Settings mSettings;                                         ///< Property: 'Settings' All the playback settings
            int mChannelCount = 1;                                      ///< Property: 'ChannelCount' Number of channels
            bool mAutoPlay = true;                                      ///< Property: 'AutoPlay' Indicates wether playback will be started on init
            ResourcePtr<EqualPowerTable> mEqualPowerTable;              ///< Property: 'EqualPowerTable' Link to EqualPowerTable used for generating the crossfade envelopes.
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;
            
        };
        

        /**
         * Instance of BufferLooper.
         * Works internally using a Polyphonic containing just two voices that are being crossfaded.
         */
        class NAPAPI BufferLooperInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
            
        public:
            BufferLooperInstance() : AudioObjectInstance() { }
            BufferLooperInstance(const std::string& name) : AudioObjectInstance(name) { }

            /**
             * Initializes the BufferLooperInstance
             * @param settings Settings of playback to be used when autoPlay is true
             * @param equalPowerTable EqualPowerTable resource used for crossfading.
             * @param channelCount Number if channels to be played back.
             * @param autoPlay True if playback should start immediately on initialization using the given settings
             * @param nodeManager NodeManager the processing is performed on.
             * @param errorState Logs initialization errors.
             * @return True on success.
             */
            bool init(BufferLooper::Settings& settings, ResourcePtr<EqualPowerTable> equalPowerTable, int channelCount, bool autoPlay, NodeManager& nodeManager, utility::ErrorState& errorState);

            // Inherited from AudioObjectInstance
            OutputPin* getOutputForChannel(int channel) override { return mPolyphonicInstance->getOutputForChannel(channel); }
            int getChannelCount() const override { return mPolyphonicInstance->getChannelCount(); }

            /**
             * Starts playback with the default settings that were passed on initialization.
             */
            void start();

            /**
             * Starts playback
             * @param settings Settings to be used for playback
             */
            void start(BufferLooper::Settings& settings);

            /**
             * Stops playback
             * @param fadeOutTime Time to fade out playback in ms.
             */
            void stop(TimeValue fadeOutTime);

            /**
             * Performs a hard reset on current playback. All voices will be stopped.
             */
            void reset();
            
        private:
            BufferLooper::Settings mSettings;
            
            Slot<EnvelopeNode&> segmentFinishedSlot = {this, &BufferLooperInstance::segmentFinished };
            void segmentFinished(EnvelopeNode& envelope);
            
            void startVoice(bool fromStart);

            std::unique_ptr<PolyphonicInstance> mPolyphonicInstance = nullptr;
            std::set<VoiceInstance*> mVoices;
            
            // private resources
            std::unique_ptr<Envelope> mEnvelope = nullptr;
            std::unique_ptr<BufferPlayer> mBufferPlayer = nullptr;
            std::unique_ptr<Multiply> mGain = nullptr;
            std::unique_ptr<Voice> mVoice = nullptr;
            std::unique_ptr<Polyphonic> mPolyphonic = nullptr;
        };
        
        
    }
    
}
