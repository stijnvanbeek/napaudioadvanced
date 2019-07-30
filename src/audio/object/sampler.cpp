#include "sampler.h"

RTTI_BEGIN_CLASS(nap::audio::Sampler::SamplerSettings)
    RTTI_PROPERTY("SampleSettings", &nap::audio::Sampler::SamplerSettings::mSampleSettings, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Envelope", &nap::audio::Sampler::SamplerSettings::mEnvelope, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::audio::Sampler)
    RTTI_PROPERTY("Settings", &nap::audio::Sampler::mSettings, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("ChannelCount", &nap::audio::Sampler::mChannelCount, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::audio::SamplerInstance)
    RTTI_FUNCTION("play", &nap::audio::SamplerInstance::play)
    RTTI_FUNCTION("stop", &nap::audio::SamplerInstance::stop)
    RTTI_FUNCTION("getSettings", &nap::audio::SamplerInstance::getSettings)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        
        bool Sampler::SamplerSettings::init(utility::ErrorState& errorState)
        {
            if (!mSampleSettings.init(errorState))
                return false;
            
            return true;
        }
        
        
        bool Sampler::init(utility::ErrorState& errorState)
        {
            return true;
        }
        
        
        std::unique_ptr<AudioObjectInstance> Sampler::createInstance(AudioService& service, utility::ErrorState& errorState)
        {
            auto instance = std::make_unique<SamplerInstance>();
            if (!instance->init(mSettings, mChannelCount, service, errorState))
                return nullptr;
            
            return instance;
        }

        
        bool SamplerInstance::init(Sampler::SamplerSettings& settings, int channelCount, AudioService& service, utility::ErrorState& errorState)
        {
            mSettings = settings;
            
            if (!mSettings.init(errorState))
                return false;
            
            mBufferLooper = std::make_unique<BufferLooper>();
            mBufferLooper->mID = "BufferLooper";
            mBufferLooper->mAutoPlay = false;
            mBufferLooper->mSettings = mSettings.mSampleSettings;
            mBufferLooper->mChannelCount = channelCount;
            if (!mBufferLooper->init(errorState))
            {
                errorState.fail("Failed to initialize BufferLooper");
                return false;
            }
            

            mEnvelope = std::make_unique<Envelope>();
            mEnvelope->mID = "Envelope";
            mEnvelope->mSegments = settings.mEnvelope;
            mEnvelope->mAutoTrigger = false;
            mEnvelope->mEqualPowerTranslate = true;
            if (!mEnvelope->init(errorState))
            {
                errorState.fail("Failed to initialize Sampler " + getName());
                return false;
            }
            
            mGain = std::make_unique<Gain>();
            mGain->mID = "Gain";
            mGain->mChannelCount = channelCount;
            mGain->mInputs.emplace_back(mBufferLooper.get());
            mGain->mInputs.emplace_back(mEnvelope.get());
            if (!mGain->init(errorState))
            {
                errorState.fail("Failed to initialize Sampler " + getName());
                return false;
            }
            
            mVoice = std::make_unique<Voice>(service);
            mVoice->mID = "Voice";
            mVoice->mObjects.emplace_back(mBufferLooper.get());
            mVoice->mObjects.emplace_back(mGain.get());
            mVoice->mObjects.emplace_back(mEnvelope.get());
            mVoice->mEnvelope = mEnvelope.get();
            mVoice->mOutput = mGain.get();
            if (!mVoice->init(errorState))
            {
                errorState.fail("Failed to initialize Sampler " + getName());
                return false;
            }
            
            mPolyphonic = std::make_unique<PolyphonicObject>();
            mPolyphonic->mID = "Polyphonic";
            mPolyphonic->mVoice = mVoice.get();
            mPolyphonic->mChannelCount = channelCount;
            mPolyphonic->mVoiceCount = 2;
            mPolyphonic->mVoiceStealing = false;
            if (!mPolyphonic->init(errorState))
            {
                errorState.fail("Failed to initialize Sampler " + getName());
                return false;
            }
            
            mPolyphonicInstance = mPolyphonic->instantiate<PolyphonicObjectInstance>(service, errorState);
            
            return true;
        }

        
        VoiceInstance* SamplerInstance::play(Sampler::SamplerSettings& settings, TimeValue duration)
        {
            auto voice = mPolyphonicInstance->findFreeVoice();
            assert(voice != nullptr);
            auto bufferLooper = voice->getObject<BufferLooperInstance>("BufferLooper");
            auto& envelope = voice->getEnvelope();
            
            bufferLooper->start(settings.mSampleSettings);
            envelope.setEnvelopeData(settings.mEnvelope);
            mPolyphonicInstance->play(voice, duration);
            
            return voice;
        }
        
        
        void SamplerInstance::stop(VoiceInstance* voice, TimeValue release)
        {
            auto& envelope = voice->getEnvelope();
            envelope.stop(release);
        }

        
        
        
    }
    
}
