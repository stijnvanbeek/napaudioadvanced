#include "synthcontroller.h"

#include <audio/object/oscillator.h>
#include <audio/object/filter.h>
#include <audio/object/multiply.h>
#include <audio/object/envelope.h>

RTTI_BEGIN_CLASS(nap::SynthController)
    RTTI_PROPERTY("AudioComponent", &nap::SynthController::mAudioComponent, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("NoteOn", &nap::SynthController::mNoteOn, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("NoteOff", &nap::SynthController::mNoteOff, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("FrequencyModulation", &nap::SynthController::mFrequencyModulation, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Voicing", &nap::SynthController::mVoicing, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("GlideTime", &nap::SynthController::mGlideTime, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("FilterCutoff", &nap::SynthController::mFilterCutoff, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("FilterResonance", &nap::SynthController::mFilterResonance, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("EnvelopeModulation", &nap::SynthController::mEnvelopeModulation, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Attack", &nap::SynthController::mAttack, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Decay", &nap::SynthController::mDecay, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Sustain", &nap::SynthController::mSustain, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Release", &nap::SynthController::mRelease, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("WaveformA", &nap::SynthController::mWaveformA, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("WaveformB", &nap::SynthController::mWaveformB, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("ReverbLevel", &nap::SynthController::mReverbLevel, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SynthControllerInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{

    bool SynthControllerInstance::init(utility::ErrorState &errorState)
    {
        mNoteOn->messageReceived.connect(noteOnSlot);
        mNoteOff->messageReceived.connect(noteOffSlot);
        mResource->mFrequencyModulation->valueChanged.connect(fmChangedSlot);
        mResource->mVoicing->valueChanged.connect(voicingChangedSlot);
        mResource->mFilterResonance->valueChanged.connect(filterResonanceChangedSlot);
        mResource->mEnvelopeModulation->valueChanged.connect(envelopeModulationChangedSlot);
        mResource->mWaveformA->valueChanged.connect(waveformAChangedSlot);
        mResource->mWaveformB->valueChanged.connect(waveformBChangedSlot);
        mResource->mReverbLevel->valueChanged.connect(reverbLevelChangedSlot);

        mPolyphonic = mAudioComponent->getObject<audio::GraphObjectInstance>()->getObject<audio::PolyphonicInstance>("Polyphonic");
        if (mPolyphonic == nullptr)
        {
            errorState.fail("Polyphonic not found");
            return false;
        }
        auto reverb = mAudioComponent->getObject<audio::GraphObjectInstance>()->getObject<audio::verb47::ReverbInstance47>("Reverb");
        reverb->getChannel(0)->setDecay(0.5f);
        reverb->getChannel(1)->setDecay(0.5f);
        auto reverbLevel = mAudioComponent->getObject<audio::GraphObjectInstance>()->getObject<audio::ControlInstance>("ReverbControl");
        reverbLevel->setValue(mResource->mReverbLevel->mValue);

        return true;
    }


    void SynthControllerInstance::update(double deltaTime)
    {
        for (auto& pair : mNoteVoices)
        {
            auto voice = pair.second;
            auto filter = voice->getObject<audio::FilterInstance>("Filter")->getChannel(0);
            auto envelope = voice->getObject<audio::EnvelopeInstance>("Envelope");
            auto cutoff = audio::mtof(mResource->mFilterCutoff->mValue);
            auto envelopeValue = envelope->getValue();
            auto freq = math::lerp(cutoff, envelopeValue * cutoff, mResource->mEnvelopeModulation->mValue);
            if (freq < 20.f)
                freq = 20.f;
            filter->setFrequency(freq);
        }
    }


    void SynthControllerInstance::noteOn(const MidiEvent& event)
    {
        if (event.getVelocity() == 0)
        {
            noteOff(event);
            return;
        }

        if (mNoteVoices.find(event.getNoteNumber()) != mNoteVoices.end())
            return;

        audio::VoiceInstance* voice = nullptr;
        float glideTime = 0.f;
        if (mResource->mVoicing->mValue == 0)
        {
            voice = mPolyphonic->findFreeVoice();
        }
        else {
            if (mMonophonicVoice == nullptr)
                voice = mPolyphonic->findFreeVoice();
            else {
                voice = mMonophonicVoice;
                glideTime = mResource->mGlideTime->mValue;
            }
        }
        auto freq = audio::mtof(event.getNoteNumber());

        voice->getObject<audio::OscillatorInstance>("OscillatorA")->selectWaveTable(mResource->mWaveformA->mValue);
        auto oscA = voice->getObject<audio::OscillatorInstance>("OscillatorA")->getChannel(0);
        oscA->setFrequency(freq, glideTime);
        oscA->setAmplitude(mResource->mFrequencyModulation->mValue, 0);

        voice->getObject<audio::OscillatorInstance>("OscillatorB")->selectWaveTable(mResource->mWaveformB->mValue);
        auto oscB = voice->getObject<audio::OscillatorInstance>("OscillatorB")->getChannel(0);
        oscB->setFrequency(freq, glideTime);

        auto filter = voice->getObject<audio::FilterInstance>("Filter")->getChannel(0);
        auto cutoffFreq = audio::mtof(math::lerp<float>(mResource->mFilterCutoff->mValue, 0.0, mResource->mEnvelopeModulation->mValue));
        if (cutoffFreq < 20.f)
            cutoffFreq = 20.f;

        auto envelope = voice->getObject<audio::EnvelopeInstance>("Envelope");
        envelope->setSegmentData(0, mResource->mAttack->mValue, 1.0, false, false, false);
        envelope->setSegmentData(1, mResource->mDecay->mValue, mResource->mSustain->mValue, false, true, true);
        envelope->setSegmentData(2, mResource->mRelease->mValue, 0, false, true, false);

        mNoteVoices[event.getNoteNumber()] = voice;
        mLastPlayedNote = event.getNoteNumber();

        if (mResource->mVoicing->mValue == 0)
        {
            filter->prepare(cutoffFreq, mResource->mFilterResonance->mValue, 1.0);
            mPolyphonic->playSection(voice, 0, 1, 0, 0);
        }
        else {
            if (mMonophonicVoice == nullptr)
            {
                filter->prepare(cutoffFreq, mResource->mFilterResonance->mValue, 1.0);
                mPolyphonic->playSection(voice, 0, 1, 0, 0);
                mMonophonicVoice = voice;
            }
            else {
                filter->setFrequency(cutoffFreq);
                filter->setResonance(mResource->mFilterResonance->mValue);
            }
        }
    }


    void SynthControllerInstance::noteOff(const MidiEvent& event)
    {
        if (mNoteVoices.find(event.getNoteNumber()) != mNoteVoices.end())
        {
            auto voice = mNoteVoices[event.getNoteNumber()];
            auto envelope = voice->getObject<audio::EnvelopeInstance>("Envelope");

            if (mResource->mVoicing->mValue == 0)
                mPolyphonic->playSection(voice, 2, 2, envelope->getValue(), 0);
            else
                if (event.getNoteNumber() == mLastPlayedNote)
                {
                    mPolyphonic->playSection(voice, 2, 2, envelope->getValue(), 0);
                    mMonophonicVoice = nullptr;
                }

           mNoteVoices.erase(event.getNoteNumber());
        }

    }


    void SynthControllerInstance::fmChanged(float value)
    {
        for (auto& pair : mNoteVoices)
        {
            auto oscA = pair.second->getObject<audio::OscillatorInstance>("OscillatorA")->getChannel(0);
            oscA->setAmplitude(value, 1);
        }
    }


    void SynthControllerInstance::voicingChanged(int)
    {
        for (auto& pair : mNoteVoices)
            mPolyphonic->stop(pair.second, 1);
        mNoteVoices.clear();
        mMonophonicVoice = nullptr;
        mLastPlayedNote = 0;
    }


    void SynthControllerInstance::filterResonanceChanged(float)
    {
        for (auto& pair : mNoteVoices)
        {
            auto filter = pair.second->getObject<audio::FilterInstance>("Filter")->getChannel(0);
            filter->setResonance(mResource->mFilterResonance->mValue);
        }
    }


    void SynthControllerInstance::envelopeModulationChanged(float)
    {
        for (auto& pair : mNoteVoices)
        {
            auto envelope = pair.second->getObject<audio::EnvelopeInstance>("Envelope");
            envelope->setSegmentData(0, mResource->mAttack->mValue, 1.0, false, false, false);
            envelope->setSegmentData(1, mResource->mDecay->mValue, mResource->mSustain->mValue, false, true, true);
            envelope->setSegmentData(2, mResource->mRelease->mValue, 0, false, true, false);
        }
    }


    void SynthControllerInstance::waveformAChanged(int value)
    {
        for (auto& pair : mNoteVoices)
        {
            auto osc = pair.second->getObject<audio::OscillatorInstance>("OscillatorA");
            osc->selectWaveTable(value);
        }
    }


    void SynthControllerInstance::waveformBChanged(int value)
    {
        for (auto& pair : mNoteVoices)
        {
            auto osc = pair.second->getObject<audio::OscillatorInstance>("OscillatorB");
            osc->selectWaveTable(value);
        }
    }


    void SynthControllerInstance::reverbLevelChanged(float value)
    {
        auto reverbLevel = mAudioComponent->getObject<audio::GraphObjectInstance>()->getObject<audio::ControlInstance>("ReverbControl");
        reverbLevel->setValue(value);
    }


}

