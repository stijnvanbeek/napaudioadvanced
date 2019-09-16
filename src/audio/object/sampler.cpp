#include "sampler.h"

RTTI_BEGIN_CLASS(nap::audio::Sampler)
    RTTI_PROPERTY("SamplerEntries", &nap::audio::Sampler::mSampleEntries, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("EnvelopeData", &nap::audio::Sampler::mEnvelopeData, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("ChannelCount", &nap::audio::Sampler::mChannelCount, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::audio::SamplerInstance)
    RTTI_FUNCTION("play", &nap::audio::SamplerInstance::play)
    RTTI_FUNCTION("stop", &nap::audio::SamplerInstance::stop)
    RTTI_FUNCTION("getEnvelopeData", &nap::audio::SamplerInstance::getEnvelopeData)
    RTTI_FUNCTION("getSamplerEntries", &nap::audio::SamplerInstance::getSamplerEntries)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        
        bool Sampler::init(utility::ErrorState& errorState)
        {
            return true;
        }
        
        
        std::unique_ptr<AudioObjectInstance> Sampler::createInstance(AudioService& service, utility::ErrorState& errorState)
        {
            auto instance = std::make_unique<SamplerInstance>();
            if (!instance->init(mSampleEntries, mEnvelopeData, mChannelCount, service, errorState))
                return nullptr;
            
            return std::move(instance);
        }

        
        bool SamplerInstance::init(Sampler::SamplerEntries& samplerEntries, EnvelopeNode::Envelope& envelopeData, int channelCount, AudioService& service, utility::ErrorState& errorState)
        {
            mSamplerEntries = samplerEntries;
            mEnvelopeData = envelopeData;
            
            for (auto& entry : mSamplerEntries)
                if (!entry.init(errorState))
                    return false;
            
            mBufferLooper = std::make_unique<BufferLooper>();
            mBufferLooper->mID = "BufferLooper";
            mBufferLooper->mAutoPlay = false;
            if (!mSamplerEntries.empty())
                mBufferLooper->mSettings = mSamplerEntries[0];
            mBufferLooper->mChannelCount = channelCount;
            if (!mBufferLooper->init(errorState))
            {
                errorState.fail("Failed to initialize BufferLooper");
                return false;
            }
            

            mEnvelope = std::make_unique<Envelope>();
            mEnvelope->mID = "Envelope";
            mEnvelope->mSegments = mEnvelopeData;
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

        
        VoiceInstance* SamplerInstance::play(unsigned int samplerEntryIndex, TimeValue duration)
        {
            if (samplerEntryIndex > mSamplerEntries.size())
                return nullptr;
            
            auto voice = mPolyphonicInstance->findFreeVoice();
            assert(voice != nullptr);
            auto bufferLooper = voice->getObject<BufferLooperInstance>("BufferLooper");
            auto& envelope = voice->getEnvelope();
            
            bufferLooper->start(mSamplerEntries[samplerEntryIndex]);
            envelope.setEnvelopeData(mEnvelopeData);
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