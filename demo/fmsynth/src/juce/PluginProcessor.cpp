#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <utility/errorstate.h>
#include <utility/fileutils.h>
#include <nap/logger.h>
#include <audio/service/audioservice.h>
#include "BinaryData.h"

#define str(s) #s
#define xstr(s) str(s)
#define symbol(s) s

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
{
	mInitialized = initializeNAP(mErrorState);
	startTimerHz(60);
}


AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
}


bool AudioPluginAudioProcessor::initializeNAP(nap::utility::ErrorState& errorState)
{
	mCore = std::make_unique<nap::Core>();
	if (!mCore->initializeEngineWithoutProjectInfo(errorState))
		return false;
	mAudioService = mCore->getService<nap::audio::AudioService>();
	if (mAudioService == nullptr)
	{
		errorState.fail("No AudioService found");
		return false;
	}
	mServices = mCore->initializeServices(errorState);
	if (!mServices->initialized())
		return false;

	mAudioService->getNodeManager().setInputChannelCount(getTotalNumInputChannels());
	mAudioService->getNodeManager().setOutputChannelCount(getTotalNumOutputChannels());

	std::string app_structure_path = xstr(APP_STRUCTURE_PATH);
	std::string data_dir = xstr(DATA_DIR);

	if (nap::utility::fileExists(app_structure_path))
	{
		nap::utility::changeDir(data_dir);
		app_structure_path = nap::utility::getFileName(app_structure_path);
		if (!mCore->getResourceManager()->loadFile(app_structure_path, errorState))
			return false;
		mCore->getResourceManager()->watchDirectory(data_dir);
	}
	else {
		std::string app_structure = symbol(APP_STRUCTURE_BINARY);
		std::vector<nap::rtti::FileLink> fileLinks;
		if (!mCore->getResourceManager()->loadJSON(app_structure, std::string(), fileLinks, errorState))
			return false;
	}

	mCore->start();
	return true;
}


//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	if (!mInitialized)
		return;

	mAudioService->getNodeManager().setSampleRate(sampleRate);
	mAudioService->getNodeManager().setInternalBufferSize(samplesPerBlock);
	mInputBuffer.resize(getTotalNumInputChannels(), samplesPerBlock);
}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

	if (!mInitialized)
		return;

	// Copy buffers to designated input buffer.
	float* inWritePtr[mInputBuffer.getChannelCount()];
	for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
		auto* channelData = buffer.getReadPointer (channel);
		memcpy(mInputBuffer[channel].data(), channelData, mInputBuffer.getSize());
		inWritePtr[channel] = buffer.getArrayOfWritePointers()[channel];
    }

	float* outWritePtr[buffer.getNumChannels()];
	for (auto i = 0; i < buffer.getNumChannels(); ++i)
		outWritePtr[i] = buffer.getArrayOfWritePointers()[i];

	mAudioService->onAudioCallback(inWritePtr, outWritePtr, buffer.getNumSamples());
}


void AudioPluginAudioProcessor::timerCallback()
{
	if (mInitialized)
		mCore->update(mUpdateFunction);
}


//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
