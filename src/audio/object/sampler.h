#pragma once

#include <audio/object/bufferlooper.h>

namespace nap
{
    
    namespace audio
    {
        
        class NAPAPI Sampler : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
        public:
            class SamplerSettings
            {
            public:
                bool init(utility::ErrorState& errorState);
                
                BufferLooper::Settings mSampleSettings;
                EnvelopeGenerator::Envelope mEnvelope;
            };
            
        public:
            Sampler() : AudioObject() { }
            
            bool init(utility::ErrorState& errorState) override;
            
            SamplerSettings mSettings;                                         ///< property: 'Settings' All the playback settings
            int mChannelCount = 1;                                      ///< property: 'ChannelCount' Number of channels
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance(AudioService& service, utility::ErrorState& errorState) override;
            
        };
        
        
        class NAPAPI SamplerInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
            
        public:
            SamplerInstance() : AudioObjectInstance() { }
            SamplerInstance(const std::string& name) : AudioObjectInstance(name) { }
            
            bool init(Sampler::SamplerSettings& settings, int channelCount, AudioService& service, utility::ErrorState& errorState);
            OutputPin* getOutputForChannel(int channel) override { return mPolyphonicInstance->getOutputForChannel(channel); }
            int getChannelCount() const override { return mPolyphonicInstance->getChannelCount(); }
            
            VoiceInstance* play(Sampler::SamplerSettings& settings, TimeValue duration);
            void stop(VoiceInstance* voice, TimeValue release = 0);
            
            Sampler::SamplerSettings& getSettings() { return mSettings; }
            
        private:
            Sampler::SamplerSettings mSettings;
            
            std::unique_ptr<PolyphonicObjectInstance> mPolyphonicInstance = nullptr;
            
            // private resources
            std::unique_ptr<Envelope> mEnvelope = nullptr;
            std::unique_ptr<BufferLooper> mBufferLooper = nullptr;
            std::unique_ptr<Gain> mGain = nullptr;
            std::unique_ptr<Voice> mVoice = nullptr;
            std::unique_ptr<PolyphonicObject> mPolyphonic = nullptr;

        };
        
        
    }
    
}
