#pragma once

#include <audio/component/audiocomponent.h>
#include <audio/core/polyphonic.h>

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
        void getDependentComponents(std::vector<rtti::TypeInfo> &types) const override {
        }

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

    private:
    };


    class SynthControllerInstance : public ComponentInstance
    {
        RTTI_ENABLE(ComponentInstance)
    public:
        SynthControllerInstance(EntityInstance &entity, Component &resource) : ComponentInstance(entity,resource)
        {
            mResource = getComponent<SynthController>();
        }


        bool init(utility::ErrorState &errorState) override
        {
            mNoteOn->messageReceived.connect(this, &SynthControllerInstance::noteOn);
            mNoteOff->messageReceived.connect(this, &SynthControllerInstance::noteOff);
            mResource->mFrequencyModulation->valueChanged.connect(this, &SynthControllerInstance::fmChanged);
            mResource->mVoicing->valueChanged.connect(this, &SynthControllerInstance::voicingChanged);
            mResource->mFilterResonance->valueChanged.connect(this, &SynthControllerInstance::filterResonanceChanged);
            mResource->mEnvelopeModulation->valueChanged.connect(this, &SynthControllerInstance::envelopeModulationChanged);
            mResource->mWaveformA->valueChanged.connect(this, &SynthControllerInstance::waveformAChanged);
            mResource->mWaveformB->valueChanged.connect(this, &SynthControllerInstance::waveformBChanged);

            mPolyphonic = mAudioComponent->getObject<audio::PolyphonicInstance>();
            if (mPolyphonic == nullptr)
            {
                errorState.fail("Polyphonic not found");
                return false;
            }

            return true;
        }


        void update(double deltaTime) override;

    private:
        void noteOn(const MidiEvent& event);
        void noteOff(const MidiEvent& event);
        void fmChanged(float);
        void voicingChanged(int);
        void filterResonanceChanged(float);
        void envelopeModulationChanged(float);
        void waveformAChanged(int);
        void waveformBChanged(int);

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