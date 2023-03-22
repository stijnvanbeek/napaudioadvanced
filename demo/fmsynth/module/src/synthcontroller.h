#pragma once

#include <audio/component/audiocomponent.h>
#include <audio/core/polyphonic.h>
#include <audio/core/graphobject.h>
#include <audio/object/reverb47.h>
#include <audio/object/control.h>

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


    class SynthController : public Component
    {
        RTTI_ENABLE(Component)
        DECLARE_COMPONENT(SynthController, SynthControllerInstance)
    public:
        ComponentPtr<audio::AudioComponent> mAudioComponent = nullptr; ///< Property: 'AudioComponent'
        ComponentPtr<MidiInputComponent> mNoteOn = nullptr;
        ComponentPtr<MidiInputComponent> mNoteOff = nullptr;
        ResourcePtr<ParameterFloat> mFrequencyModulation = nullptr;
        ResourcePtr<ParameterOptionList> mVoicing = nullptr;
        ResourcePtr<ParameterFloat> mGlideTime  = nullptr;
        ResourcePtr<ParameterFloat> mFilterCutoff = nullptr;
        ResourcePtr<ParameterFloat> mFilterResonance = nullptr;
        ResourcePtr<ParameterFloat> mEnvelopeModulation = nullptr;
        ResourcePtr<ParameterFloat> mAttack = nullptr;
        ResourcePtr<ParameterFloat> mDecay = nullptr;
        ResourcePtr<ParameterFloat> mSustain = nullptr;
        ResourcePtr<ParameterFloat> mRelease = nullptr;
        ResourcePtr<ParameterOptionList> mWaveformA = nullptr;
        ResourcePtr<ParameterOptionList> mWaveformB = nullptr;
        ResourcePtr<ParameterFloat> mReverbLevel = nullptr;
    };


    class SynthControllerInstance : public ComponentInstance
    {
        RTTI_ENABLE(ComponentInstance)
    public:
        SynthControllerInstance(EntityInstance &entity, Component &resource) : ComponentInstance(entity,resource)
        {
            mResource = getComponent<SynthController>();
        }

        // Inherited from ComponentInstance
        bool init(utility::ErrorState &errorState) override;
        void update(double deltaTime) override;

    private:
        Slot<const MidiEvent&> noteOnSlot = { this, &SynthControllerInstance::noteOn };
        void noteOn(const MidiEvent& event);
        Slot<const MidiEvent&> noteOffSlot = { this, &SynthControllerInstance::noteOff };
        void noteOff(const MidiEvent& event);
        Slot<float> fmChangedSlot = { this, &SynthControllerInstance::fmChanged };
        void fmChanged(float);
        Slot<int> voicingChangedSlot = { this, &SynthControllerInstance::voicingChanged };
        void voicingChanged(int);
        Slot<float> filterResonanceChangedSlot = { this, &SynthControllerInstance::filterResonanceChanged };
        void filterResonanceChanged(float);
        Slot<float> envelopeModulationChangedSlot = { this, &SynthControllerInstance::envelopeModulationChanged };
        void envelopeModulationChanged(float);
        Slot<int> waveformAChangedSlot = { this, &SynthControllerInstance::waveformAChanged };
        void waveformAChanged(int);
        Slot<int> waveformBChangedSlot = { this, &SynthControllerInstance::waveformBChanged };
        void waveformBChanged(int);
        Slot<float> reverbLevelChangedSlot = { this, &SynthControllerInstance::reverbLevelChanged };
        void reverbLevelChanged(float);

        audio::PolyphonicInstance* mPolyphonic = nullptr;
        int mLastPlayedNote = 0;
        audio::VoiceInstance* mMonophonicVoice = nullptr;
        std::map<int, audio::VoiceInstance*> mNoteVoices;
        SynthController* mResource = nullptr;

        ComponentInstancePtr<audio::AudioComponent> mAudioComponent = { this, &SynthController::mAudioComponent };
        ComponentInstancePtr<MidiInputComponent> mNoteOn = { this, &SynthController::mNoteOn };
        ComponentInstancePtr<MidiInputComponent> mNoteOff = { this, &SynthController::mNoteOff };
    };

}