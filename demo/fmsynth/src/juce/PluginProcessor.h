#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <nap/core.h>
#include <audio/service/audioservice.h>

//==============================================================================
class AudioPluginAudioProcessor  : public juce::AudioProcessor, public juce::Timer
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	nap::Core& getCore() { return *mCore; }

	void timerCallback() override;
	bool initializeNAP(nap::utility::ErrorState& errorState);

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)

    std::unique_ptr<nap::Core> mCore;
	nap::audio::AudioService* mAudioService = nullptr;
	nap::Core::ServicesHandle mServices;
	nap::audio::MultiSampleBuffer mInputBuffer;
	std::function<void(double)> mUpdateFunction = [](double){};
	nap::utility::ErrorState mErrorState;
	bool mInitialized = false;
};
