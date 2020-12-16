#pragma once

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/nodeobject.h>
#include <audio/node/oscillatornode.h>
#include <audio/utility/safeptr.h>

namespace nap
{
    
    namespace audio
    {

		// Forward declarations
		class OscillatorInstance;


        class NAPAPI WaveTableResource : public Resource
		{
            RTTI_ENABLE(Resource)

        public:
            WaveTableResource(NodeManager& nodeManager) : Resource(), mNodeManager(nodeManager) { }
            bool init(utility::ErrorState& errorState);

            int mSize = 2048; ///< Property: 'Size' Size of the wavetable. Has to be a power of two.
            int mNumberOfBands = 100; ///< Property: 'NumberOfBands' Number of bands used for band limiting
            WaveTable::Waveform mWaveform = WaveTable::Waveform::Sine; ///< Property: 'Waveform' Waveform of the wave table.

            SafePtr<WaveTable> getWave() { return mWave.get(); }

        private:
            SafeOwner<WaveTable> mWave = nullptr;
            NodeManager& mNodeManager;
        };


		/**
         * Multichannel oscillator object.
         */
        class NAPAPI Oscillator : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
            
        public:
            std::vector<ControllerValue> mFrequency = { 220.f }; ///< property: 'Frequency' array of frequency values that will be mapped on the oscillators on each channel
            std::vector<ControllerValue> mAmplitude = { 1.f }; ///< property: 'Amplitude' array of amplitude values that will be mapped on the oscillators on each channel
            ResourcePtr<AudioObject> mFmInput = nullptr; ///< property: 'FmInput' audio object of which the outputs will modulate the frequencies of the oscillators on each channel.
            std::vector<ResourcePtr<WaveTableResource>> mWaveTables; ///< property: 'WaveTables' Pointers to a collection of different wave table resources that can be chosen from at runtime.
			int mWaveTableSelection = 0; ///< property: 'WaveTableIndex' Selection from the list of wavetables that will be used on initialization.
			int mChannelCount = 1; ///< property: 'ChannelCount' Number of channels
            
        private:
			std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;
        };


		class OscillatorInstance : public AudioObjectInstance
		{
			RTTI_ENABLE(AudioObjectInstance)

		public:
			OscillatorInstance() = default;
			OscillatorInstance(const std::string& name) : AudioObjectInstance(name) { }

			bool init(int channelCount, NodeManager& nodeManager, utility::ErrorState& errorState);
			void addWaveTable(const SafePtr<WaveTable>& waveTable) { mWaveTables.emplace_back(waveTable); }
			void selectWaveTable(int index);
			int getWaveTableCount() const { return mWaveTables.size(); }

			int getChannelCount() const override { return mNodes.size(); }
			OutputPin* getOutputForChannel(int channel) override { return &mNodes[channel]->output; }
			OscillatorNode* getChannel(int channel) { return mNodes[channel].getRaw(); }

		private:
			std::vector<SafePtr<WaveTable>> mWaveTables;
			std::vector<SafeOwner<OscillatorNode>> mNodes;
		};


		using WaveTableResourceObjectCreator = rtti::ObjectCreator<WaveTableResource, NodeManager>;

    }
    
}
