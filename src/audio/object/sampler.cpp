#include "sampler.h"

RTTI_BEGIN_CLASS(nap::audio::SamplePlayer)
    RTTI_PROPERTY("SamplerEntries", &nap::audio::SamplePlayer::mSampleEntries, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("EnvelopeData", &nap::audio::SamplePlayer::mEnvelopeData, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("ChannelCount", &nap::audio::SamplePlayer::mChannelCount, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("VoiceCount", &nap::audio::SamplePlayer::mVoiceCount, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("EqualPowerTable", &nap::audio::SamplePlayer::mEqualPowerTable, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::audio::SamplePlayerInstance)
    RTTI_FUNCTION("play", &nap::audio::SamplePlayerInstance::play)
    RTTI_FUNCTION("stop", &nap::audio::SamplePlayerInstance::stop)
    RTTI_FUNCTION("getEnvelopeData", &nap::audio::SamplePlayerInstance::getEnvelopeData)
    RTTI_FUNCTION("getSamplerEntries", &nap::audio::SamplePlayerInstance::getSamplerEntries)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        
        bool SamplePlayer::init(utility::ErrorState& errorState)
        {
            return true;
        }
        
        
        std::unique_ptr<AudioObjectInstance> SamplePlayer::createInstance(NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            auto instance = std::make_unique<SamplePlayerInstance>();
            if (!instance->init(mSampleEntries, mEqualPowerTable, mEnvelopeData, mChannelCount, mVoiceCount, nodeManager, errorState))
                return nullptr;
            
            return std::move(instance);
        }

        
        bool SamplePlayerInstance::init(SamplePlayer::SamplerEntries& samplerEntries, ResourcePtr<EqualPowerTable> equalPowerTable, EnvelopeNode::Envelope& envelopeData, int channelCount, int voiceCount, NodeManager& nodeManager, utility::ErrorState& errorState)
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
            mBufferLooper->mEqualPowerTable = equalPowerTable;
            if (!mBufferLooper->init(errorState))
            {
                errorState.fail("Failed to initialize BufferLooper");
                return false;
            }
            

            mEnvelope = std::make_unique<Envelope>();
            mEnvelope->mID = "Envelope";
            mEnvelope->mSegments = mEnvelopeData;
            mEnvelope->mAutoTrigger = false;
            mEnvelope->mEqualPowerTable = equalPowerTable;
            if (!mEnvelope->init(errorState))
            {
                errorState.fail("Failed to initialize Sampler " + getName());
                return false;
            }
            
            mGain = std::make_unique<Multiply>();
            mGain->mID = "Gain";
            mGain->mChannelCount = channelCount;
            mGain->mInputs.emplace_back(mBufferLooper.get());
            mGain->mInputs.emplace_back(mEnvelope.get());
            if (!mGain->init(errorState))
            {
                errorState.fail("Failed to initialize Sampler " + getName());
                return false;
            }
            
            mVoice = std::make_unique<Voice>();
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
            
            mPolyphonic = std::make_unique<Polyphonic>();
            mPolyphonic->mID = "Polyphonic";
            mPolyphonic->mVoice = mVoice.get();
            mPolyphonic->mChannelCount = channelCount;
            mPolyphonic->mVoiceCount = voiceCount;
            mPolyphonic->mVoiceStealing = false;
            if (!mPolyphonic->init(errorState))
            {
                errorState.fail("Failed to initialize Sampler " + getName());
                return false;
            }

            mPolyphonicInstance = mPolyphonic->instantiate<PolyphonicInstance>(nodeManager, errorState);
            if (mPolyphonicInstance == nullptr)
            {
                errorState.fail("Failed to instantiate polyphonic.");
                return false;
            }
            
            return true;
        }

        
        VoiceInstance* SamplePlayerInstance::play(unsigned int samplerEntryIndex, TimeValue duration)
        {
            if (samplerEntryIndex > mSamplerEntries.size())
                return nullptr;

            int busyVoiceCount = mPolyphonicInstance->getBusyVoiceCount();
            Logger::info("Busy voice count %i", busyVoiceCount);
            
            auto voice = mPolyphonicInstance->findFreeVoice();
            assert(voice != nullptr);
            auto bufferLooper = voice->getObject<BufferLooperInstance>("BufferLooper");
            auto& envelope = voice->getEnvelope();

            bufferLooper->reset();
            bufferLooper->start(mSamplerEntries[samplerEntryIndex]);
            envelope.setEnvelopeData(mEnvelopeData);

            mPolyphonicInstance->play(voice, duration);
            
            return voice;
        }


        void SamplePlayerInstance::stop(VoiceInstance* voice, TimeValue release)
        {
            auto bufferLooper = voice->getObject<BufferLooperInstance>("BufferLooper");
            auto& envelope = voice->getEnvelope();
            if (release == 0.f)
                envelope.stop(1.f);
            else
                envelope.stop(release);
//            bufferLooper->stop();
        }

        
        
        
    }
    
}
