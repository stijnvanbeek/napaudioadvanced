#include "synthcontroller.h"

#include <audio/object/multiply.h>

// RTTI definitions

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
    RTTI_PROPERTY("Waveform", &nap::SynthController::mWaveform, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("ReverbLevel", &nap::SynthController::mReverbLevel, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SynthControllerInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{

    bool SynthControllerInstance::init(utility::ErrorState &errorState)
    {
        // Connect midi input components capturing note events to their corresponding slots
        mNoteOn->messageReceived.connect(noteOnSlot);
        mNoteOff->messageReceived.connect(noteOffSlot);

        // Connect slots handling certain parameter changes
        mResource->mFrequencyModulation->valueChanged.connect(fmChangedSlot);
        mResource->mVoicing->valueChanged.connect(voicingChangedSlot);
        mResource->mFilterResonance->valueChanged.connect(filterResonanceChangedSlot);
        mResource->mWaveform->valueChanged.connect(waveformChangedSlot);
        mResource->mReverbLevel->valueChanged.connect(reverbLevelChangedSlot);

        // Find Graph in sibling AudioComponent
        auto graph =  mAudioComponent->getObject<audio::GraphObjectInstance>();
        if (graph == nullptr)
        {
            errorState.fail("Audio graph not found");
            return false;
        }

        // Find Polyphonic in Graph
        mPolyphonic = graph->getObject<audio::PolyphonicInstance>("Polyphonic");
        if (mPolyphonic == nullptr)
        {
            errorState.fail("Polyphonic not found");
            return false;
        }

        // Find reverb in Graph
        auto reverb = graph->getObject<audio::verb47::ReverbInstance47>("Reverb");
        if (reverb == nullptr)
        {
            errorState.fail("Reverb not found");
            return false;
        }

        // Find reverb level control
        mReverbLevelControl = graph->getObject<audio::ControlInstance>("ReverbControl");
        if (mReverbLevelControl == nullptr)
        {
            errorState.fail("Reverb level control not found");
            return false;
        }

        // Adjust reverb settings
        reverb->getChannel(0)->setDecay(0.5f);
        reverb->getChannel(1)->setDecay(0.5f);
        mReverbLevelControl->setValue(mResource->mReverbLevel->mValue);

        // Initialize the maps containing audio objects from the voice DSP mapped to their voices
        if (!mPolyphonic->getObjectMap("Filter", mFilters, errorState))
            return false;
        if (!mPolyphonic->getObjectMap("ModulatorOscillator", mModulatorOscillators, errorState))
            return false;
        if (!mPolyphonic->getObjectMap("CarrierOscillator", mCarrierOscillators, errorState))
            return false;
        if (!mPolyphonic->getObjectMap("Envelope", mEnvelopes, errorState))
            return false;

        return true;
    }


    void SynthControllerInstance::update(double deltaTime)
    {
        // Iterate through all playing voices to update their filter cutoff frequencies
        // This is done on update because the cutoff frequency modulates with the ADSR envelope value
        for (auto& pair : mNoteVoices)
        {
            // Grab the filter and the envelope for the playing voice
            auto voice = pair.second;
            auto filter = mFilters[voice]->getChannel(0);
            auto envelope = mEnvelopes[voice];

            auto cutoff = audio::mtof(mResource->mFilterCutoff->mValue); // Convert cutoff frequency parameter midi note value to Hz
            auto envelopeValue = envelope->getValue(); // Get current ADSR envelope output
            auto freq = math::lerp(cutoff, envelopeValue * cutoff, mResource->mEnvelopeModulation->mValue); // Combine cutoff parameter and ADSR value using envelope modulation parameter
            // Make sure cutoff frequency value doesn't blow up in subsonic range and set the filter frequency
            if (freq < 20.f)
                freq = 20.f;
            filter->setFrequency(freq);
        }
    }


    void SynthControllerInstance::noteOn(const MidiEvent& event)
    {
        // A note on event with velocity zero is handled as a note off
        if (event.getVelocity() == 0)
        {
            noteOff(event);
            return;
        }

        // If there is already a voice playing for this midi note the incoming note event is ignored
        if (mNoteVoices.find(event.getNoteNumber()) != mNoteVoices.end())
            return;

        // Determine the voice and the glide time for the new note
        audio::VoiceInstance* voice = nullptr;
        float glideTime = 0.f;
        if (mResource->mVoicing->mValue == 0)
        {
            // In polyphonic mode a new note is always played on a free voice.
            // The glide time remains zero because gliding only occurs in monophonic mode
            voice = mPolyphonic->findFreeVoice();
        }
        else {
            // In monophonic mode check if there is already a monophonic voice playing.
            // If not, find a free voice to start the monophonic line.
            if (mMonophonicVoice == nullptr)
                voice = mPolyphonic->findFreeVoice();
            else {
                voice = mMonophonicVoice;
                glideTime = mResource->mGlideTime->mValue; // The glide time is updated from the parameter value.
            }
        }

        // Calculate the new note's frequency in Hz
        auto freq = audio::mtof(event.getNoteNumber());

        // Update the modulator oscillator. This is the oscillator that modulates the frequency of the audible oscillator. (the carrier)
        auto oscA = mModulatorOscillators[voice]->getChannel(0); // Because the oscillator is mono we operate on channel 0
        oscA->setFrequency(freq, glideTime);
        oscA->setAmplitude(mResource->mFrequencyModulation->mValue, 0); // The amplitude of the modulator oscillator is determined by the frequency modulation amount.

        // Update the carrier oscillator. This is the oscillator that is generating the audible signal.
        mCarrierOscillators[voice]->selectWaveTable(mResource->mWaveform->mValue);
        auto oscB = mCarrierOscillators[voice]->getChannel(0);  // Because the oscillator is mono we operate on channel 0
        oscB->setFrequency(freq, glideTime);

        auto filter = mFilters[voice]->getChannel(0); // Because the filter is mono we operate on channel 0
        // Calculate the cutoff frequency of the filter, avoid low subsonic cutoff frequencies
        auto cutoffFreq = audio::mtof(math::lerp<float>(mResource->mFilterCutoff->mValue, 0.0, mResource->mEnvelopeModulation->mValue));
        if (cutoffFreq < 20.f)
            cutoffFreq = 20.f;

        // Update the ADSR envelope's segment data
        auto envelope = mEnvelopes[voice];
        envelope->setSegmentData(0, mResource->mAttack->mValue, 1.0, false, false, false);
        envelope->setSegmentData(1, mResource->mDecay->mValue, mResource->mSustain->mValue, false, true, true);
        envelope->setSegmentData(2, mResource->mRelease->mValue, 0, false, true, false);

        // Update the note voice map and last played note variable for internal administration
        mNoteVoices[event.getNoteNumber()] = voice;
        mLastPlayedNote = event.getNoteNumber();

        if (mResource->mVoicing->mValue == 0)
        {
            // In polyphonic mode: prepare the filter for playback and play the attack and decay sections of the envelope
            filter->prepare(cutoffFreq, mResource->mFilterResonance->mValue, 1.0);
            mPolyphonic->playSection(voice, 0, 1, 0, 0);
        }
        else {
            if (mMonophonicVoice == nullptr)
            {
                // In monophonic mode, if we start a new line: prepare the filter for playback and play the attack and decay sections of the envelope
                filter->prepare(cutoffFreq, mResource->mFilterResonance->mValue, 1.0);
                mPolyphonic->playSection(voice, 0, 1, 0, 0);
                mMonophonicVoice = voice; // Update the pointer to the voice playing the monophonic line
            }
            else {
                // If we are continuing an already playing monophonic line, just update the filter of the playing voice.
                filter->setFrequency(cutoffFreq);
                filter->setResonance(mResource->mFilterResonance->mValue);
            }
        }
    }


    void SynthControllerInstance::noteOff(const MidiEvent& event)
    {
        // I there is no playing voice found for the note off event it can be ignored
        if (mNoteVoices.find(event.getNoteNumber()) != mNoteVoices.end())
        {
            auto voice = mNoteVoices[event.getNoteNumber()];
            auto envelope = mEnvelopes[voice];

            if (mResource->mVoicing->mValue == 0)
                // In polyphonic mode play the release section of the playing voice
                mPolyphonic->playSection(voice, 2, 2, envelope->getValue(), 0);
            else
                // In monophonic mode, only respond if the released midi note corresponds to the last played note
                if (event.getNoteNumber() == mLastPlayedNote)
                {
                    // Play the release section of the envelope and clear the monophonic voice variable
                    mPolyphonic->playSection(voice, 2, 2, envelope->getValue(), 0);
                    mMonophonicVoice = nullptr;
                }

           // Erase the note number from the voice mapping
           mNoteVoices.erase(event.getNoteNumber());
        }

    }


    void SynthControllerInstance::fmChanged(float value)
    {
        // Update the frequency modulation amount for the modulator oscillators in all playing voices
        for (auto& pair : mNoteVoices)
        {
            auto oscA = mModulatorOscillators[pair.second]->getChannel(0);
            oscA->setAmplitude(value, 1);
        }
    }


    void SynthControllerInstance::voicingChanged(int)
    {
        // Stop all playback when the voicing parameter changes
        for (auto& pair : mNoteVoices)
            mPolyphonic->stop(pair.second, 1);
        mNoteVoices.clear();
        mMonophonicVoice = nullptr;
        mLastPlayedNote = 0;
    }


    void SynthControllerInstance::filterResonanceChanged(float)
    {
        // Update the resonance of the filters in all playing voices
        for (auto& pair : mNoteVoices)
        {
            auto filter = mFilters[pair.second]->getChannel(0);
            filter->setResonance(mResource->mFilterResonance->mValue);
        }
    }


    void SynthControllerInstance::waveformChanged(int value)
    {
        // Update the waveform for the carrier oscillators in all playing voices
        for (auto& pair : mNoteVoices)
        {
            auto osc = mCarrierOscillators[pair.second];
            osc->selectWaveTable(value);
        }
    }


    void SynthControllerInstance::reverbLevelChanged(float value)
    {
        // Update the level of the reverberated signal
        mReverbLevelControl->setValue(value);
    }


}

