/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <audio/object/bufferlooper.h>

namespace nap
{
    
    namespace audio
    {

        /**
         * Object that plays back samples along with some metadata about start point, loop points and transposition
         */
        class NAPAPI SamplePlayer : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
            
        public:
            using SamplerEntries = std::vector<BufferLooper::Settings>;
            
        public:
			SamplePlayer() : AudioObject() { }

			// Inherited from AudioObject
            bool init(utility::ErrorState& errorState) override;
            
            SamplerEntries mSampleEntries;                              ///< Property: 'SampleEntries' Default set of different playback settings
            EnvelopeNode::Envelope mEnvelopeData;                       ///< Property: 'Envelope' Default envelope settings
            int mChannelCount = 1;                                      ///< Property: 'ChannelCount' Number of channels
            int mVoiceCount = 10;                                       ///< Property: 'VoiceCount' Number of voices in the pool.
            ResourcePtr<EqualPowerTable> mEqualPowerTable = nullptr;    ///< Property: 'EqualPowerTable' Pointer to EqualPowerTable that will be used by crossfades and envelopes.
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;
            
        };
        

        /**
         * Instance of SamplePlayer
         */
        class NAPAPI SamplePlayerInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
            
        public:
			SamplePlayerInstance() : AudioObjectInstance() { }
			SamplePlayerInstance(const std::string& name) : AudioObjectInstance(name) { }

            /**
             * @param sampleEntries Sample entries containing playback metadata that can be played by this sampler
             * @param equalPowerTable EqualPowerTable that will be used by crossfades and envelopes
             * @param envelopeData Default envelope data
             * @param channelCount Number of output channels of the sampler
             * @param nodeManager NodeManager this sampler runs on
             * @param errorState Contains error information if the init() fails
             * @return True on success
             */
            bool init(SamplePlayer::SamplerEntries& sampleEntries, ResourcePtr<EqualPowerTable> equalPowerTable, EnvelopeNode::Envelope& envelopeData, int channelCount, int voiceCount, NodeManager& nodeManager, utility::ErrorState& errorState);

            // Inhrited from AudioObjectInstance
            OutputPin* getOutputForChannel(int channel) override { return mPolyphonicInstance->getOutputForChannel(channel); }
            int getChannelCount() const override { return mPolyphonicInstance->getChannelCount(); }

             /**
              * Plays back sampler entry with given index for given duration.
              * @param samplerEntryIndex Index of the sampler entry metadata for playback
              * @param duration Duration of the playback
              * @return Voice that is playing back the entry
              */
            VoiceInstance* play(unsigned int samplerEntryIndex, TimeValue duration);

            /**
             * Plays a sampler entry, but specify the section of the envelope that will be played.
             * @param samplerEntryIndex Index of the sampler entry meta data
             * @param startSegment Starting segment of the envelope data that is triggered
             * @param endSegment Ending segment of the envelope data
             * @param startValue Initial value of the envelope output when the startSegment is triggered.
             * @param totalDuration The total duration of the playback event.
             * @return The voice playing back the section
             */
            VoiceInstance* playSection(unsigned int samplerEntryIndex, int startSegment, int endSegment, ControllerValue startValue = 0, TimeValue totalDuration = 0);

            /**
             * Stops a voice that is playing back a sample event
             * @param voice The voice that will be stopped
             * @param release Time in ms of the fadeout
             */
            void stop(VoiceInstance* voice, TimeValue release = 0);

            /**
             * @return all the available sampler entries. Non editable.
             */
            const SamplePlayer::SamplerEntries& getSamplerEntries() const { return mSamplerEntries; }

            /**
             * @return the envelope data that will be passed to the next voice to be played. Result can be edited to affect the next play() statement.
             */
            EnvelopeNode::Envelope& getEnvelopeData() { return mEnvelopeData; }
            
        private:
			SamplePlayer::SamplerEntries mSamplerEntries;
            EnvelopeNode::Envelope mEnvelopeData;
            
            std::unique_ptr<PolyphonicInstance> mPolyphonicInstance = nullptr;
            
            // private resources
            std::unique_ptr<Envelope> mEnvelope = nullptr;
            std::unique_ptr<BufferLooper> mBufferLooper = nullptr;
            std::unique_ptr<Multiply> mGain = nullptr;
            std::unique_ptr<Voice> mVoice = nullptr;
            std::unique_ptr<Polyphonic> mPolyphonic = nullptr;
        };
        
        
    }
    
}
