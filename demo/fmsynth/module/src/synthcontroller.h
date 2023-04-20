#pragma once

#include <audio/component/audiocomponent.h>
#include <audio/core/polyphonic.h>
#include <audio/core/graphobject.h>
#include <audio/object/reverb47.h>
#include <audio/object/control.h>
#include <audio/object/oscillator.h>
#include <audio/object/filter.h>
#include <audio/object/envelope.h>

#include <midiinputcomponent.h>

#include <parametergroup.h>
#include <parameternumeric.h>
#include <parametersimple.h>
#include <parameteroptionlist.h>
#include <entity.h>
#include <componentptr.h>

#include <map>

namespace nap
{

    // Forward declarations
    class SynthControllerInstance;


    /**
     * This component contains all the logic about the polyphonic FM synthesizer.
     * It responds to midi input and parameter changes by manipulating and playing voices within a Polyphonic object.
     * Each voice of the polyphonic synth has a carrier oscillator that can be set to sine, saw or squarewave mode and a modulating lowpassfilter.
     * The frequency of the carrier oscillator can be modulated by another modulator oscillator.
     */
    class SynthController : public Component
    {
        RTTI_ENABLE(Component)
        DECLARE_COMPONENT(SynthController, SynthControllerInstance)
    public:
        ComponentPtr<audio::AudioComponent> mAudioComponent = nullptr; ///< Property: 'AudioComponent' Points to the AudioComponent that manages the DSP containing the Polyphonic object.
        ComponentPtr<MidiInputComponent> mNoteOn = nullptr; ///< Property: 'NoteOn' MidiInputComponent that captures all note on events.
        ComponentPtr<MidiInputComponent> mNoteOff = nullptr; ///< Property: 'NoteOff' MidiInputComponent that captures all note off events.
        ResourcePtr<ParameterFloat> mFrequencyModulation = nullptr; ///< Property: 'FrequencyModulation' Parameter that controls the frequency modulation amount
        ResourcePtr<ParameterOptionList> mVoicing = nullptr; ///< Property: 'Voicing' Parameter that indicates of the synth is playing in polyphonic or monophonic mode. Monophonic meaning that the already playing note is gliding to the new played note.
        ResourcePtr<ParameterFloat> mGlideTime  = nullptr; ///< Property: 'GlideTime' Parameter indicating the glide time in milliseconds between two notes playing in overlap in monophonic mode.
        ResourcePtr<ParameterFloat> mFilterCutoff = nullptr; ///< Property: 'FilterCutoff' Parameter indicating the cutoff frequency in of the filter expressed in a floating point midi note number.
        ResourcePtr<ParameterFloat> mFilterResonance = nullptr; ///< Property: 'Resonance' Parameter indicating the resonance value of the filter.
        ResourcePtr<ParameterFloat> mEnvelopeModulation = nullptr; ///< Property: 'EnvelopeModulation' Parameter indicating the amount of envelope modulation of the filter cutoff frequency.
        ResourcePtr<ParameterFloat> mAttack = nullptr; ///< Property: 'Attack' Attack parameter in ms of the ADSR envelope
        ResourcePtr<ParameterFloat> mDecay = nullptr; ///< Property: 'Decay' Decay parameter in ms of the ADSR envelope
        ResourcePtr<ParameterFloat> mSustain = nullptr; ///< Property: 'Sustain' Sustain amplitude of the ADSR envelope
        ResourcePtr<ParameterFloat> mRelease = nullptr; ///< Property: 'Release' Release parameter in ms of the ADSR envelope
        ResourcePtr<ParameterOptionList> mWaveform = nullptr; ///< Property: 'Waveform' Waveform of the carrier oscillator. 0: sine, 1: saw, 2: square
        ResourcePtr<ParameterFloat> mReverbLevel = nullptr; ///< Property: 'ReverbLevel' Level of the reverberated signal.
    };


    /**
     * Instance of SynthController component
     */
    class SynthControllerInstance : public ComponentInstance
    {
        RTTI_ENABLE(ComponentInstance)
    public:
        // Constructor
        SynthControllerInstance(EntityInstance &entity, Component &resource) : ComponentInstance(entity,resource)
        {
            mResource = getComponent<SynthController>();
        }

        // Inherited from ComponentInstance
        bool init(utility::ErrorState &errorState) override;
        void update(double deltaTime) override;

    private:
        // Slots to respond to incoming midi note events
        Slot<const MidiEvent&> noteOnSlot = { this, &SynthControllerInstance::noteOn };
        void noteOn(const MidiEvent& event);
        Slot<const MidiEvent&> noteOffSlot = { this, &SynthControllerInstance::noteOff };
        void noteOff(const MidiEvent& event);

        // Slots to respond to parameter changes
        Slot<float> fmChangedSlot = { this, &SynthControllerInstance::fmChanged };
        void fmChanged(float);
        Slot<int> voicingChangedSlot = { this, &SynthControllerInstance::voicingChanged };
        void voicingChanged(int);
        Slot<float> filterResonanceChangedSlot = { this, &SynthControllerInstance::filterResonanceChanged };
        void filterResonanceChanged(float);
        Slot<int> waveformChangedSlot = { this, &SynthControllerInstance::waveformChanged };
        void waveformChanged(int);
        Slot<float> reverbLevelChangedSlot = { this, &SynthControllerInstance::reverbLevelChanged };
        void reverbLevelChanged(float);

        SynthController* mResource = nullptr; // Pointer to this component's resource
        audio::PolyphonicInstance* mPolyphonic = nullptr; // Polyphonic object performing the DSP for the synthesizer's voices

        int mLastPlayedNote = 0; // The midi note number of the last played note. This is saved for monophonic mode.
        audio::VoiceInstance* mMonophonicVoice = nullptr; // In monophonic mode there is only one single voice playing, this is a pointer to that voice.
        std::map<int, audio::VoiceInstance*> mNoteVoices; // In polyphonic mode this map contains for each note that is currently being played a pair of the midi note number and a pointer to the voice playing this note.

        audio::PolyphonicInstance::ObjectMap<audio::EnvelopeInstance> mEnvelopes; // This is a helper map with the envelope object for each voice mapped to its voice for lookup purposes
        audio::PolyphonicInstance::ObjectMap<audio::OscillatorInstance> mModulatorOscillators; // This is a helper map with the modulator oscillator object for each voice mapped to its voice  for lookup purposes
        audio::PolyphonicInstance::ObjectMap<audio::OscillatorInstance> mCarrierOscillators; // This is a helper map with the carrier oscillator object for each voice mapped to its voice for lookup purposes
        audio::PolyphonicInstance::ObjectMap<audio::FilterInstance> mFilters; // This is a helper map with the filter object for each voice mapped to its voice for lookup purposes
        audio::ControlInstance* mReverbLevelControl = nullptr; // Pointer to the control object that controls the level of the reverberated signal

        ComponentInstancePtr<audio::AudioComponent> mAudioComponent = { this, &SynthController::mAudioComponent }; // Sibling AudioComponent that contains all the DSP objects in a audio::GraphObject
        ComponentInstancePtr<MidiInputComponent> mNoteOn = { this, &SynthController::mNoteOn }; // MidiInputComponent capturing all midi note events
        ComponentInstancePtr<MidiInputComponent> mNoteOff = { this, &SynthController::mNoteOff }; // MidiInputComponent capturing all midi note off events
    };

}