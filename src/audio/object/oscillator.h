/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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


		/**
		 * Resource wrapper around WaveTable object.
		 */
        class NAPAPI WaveTableResource : public Resource
		{
            RTTI_ENABLE(Resource)

        public:
            WaveTableResource(NodeManager& nodeManager) : Resource(), mNodeManager(nodeManager) { }
            bool init(utility::ErrorState& errorState);

            int mSize = 2048;                                          ///< Property: 'Size' Size of the wavetable. Has to be a power of two.
            int mNumberOfBands = 100;                                  ///< Property: 'NumberOfBands' Number of bands used for band limiting
            WaveTable::Waveform mWaveform = WaveTable::Waveform::Sine; ///< Property: 'Waveform' Waveform of the wave table.

            /**
             * @return Pointer to the managed WaveTable object.
             */
            SafePtr<WaveTable> getWave() { return mWave.get(); }

        private:
            SafeOwner<WaveTable> mWave = nullptr;
            NodeManager& mNodeManager;
        };


		/**
         * Multichannel oscillator object.
         * The Oscillator basically repeats a waveform (default a sinewave) in a frequency that can be modulated.
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


        /**
         * Instance of Oscillator
         */
		class OscillatorInstance : public AudioObjectInstance
		{
			RTTI_ENABLE(AudioObjectInstance)

		public:
			OscillatorInstance() = default;
			OscillatorInstance(const std::string& name) : AudioObjectInstance(name) { }

			/**
			 * Initialize the OscillatorInstance
			 * @param channelCount Number of channels
			 * @param nodeManager The NodeManager this object will process on
			 * @param errorState Logs errors during initialization
			 * @return True on success
			 */
			bool init(int channelCount, NodeManager& nodeManager, utility::ErrorState& errorState);

			/**
			 * Adds a WaveTable to read from. The wave tabke can be selected using the selectWaveTable() function.
			 * @param waveTable
			 * @return The index of the wavetable that can be used to select it using selectWaveTable()
			 */
			int addWaveTable(const SafePtr<WaveTable>& waveTable);

			/**
			 * Selects a wavetable that has been added using addWaveTable()
			 * @param index Index of the wavetable.
			 */
			void selectWaveTable(int index);

			/**
			 * @return The number of wavetables that can be chosen from
			 */
			int getWaveTableCount() const { return mWaveTables.size(); }

			// Inherited from AudioObjectInstance
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
