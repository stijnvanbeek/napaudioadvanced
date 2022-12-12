/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "oscillator.h"

RTTI_BEGIN_ENUM(nap::audio::WaveTable::Waveform)
	RTTI_ENUM_VALUE(nap::audio::WaveTable::Waveform::Sine, "Sine"),
	RTTI_ENUM_VALUE(nap::audio::WaveTable::Waveform::Saw, "Saw"),
	RTTI_ENUM_VALUE(nap::audio::WaveTable::Waveform::Square, "Square"),
	RTTI_ENUM_VALUE(nap::audio::WaveTable::Waveform::Triangle, "Triangle")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::WaveTableResource)
    RTTI_PROPERTY("Size", &nap::audio::WaveTableResource::mSize, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Waveform", &nap::audio::WaveTableResource::mWaveform, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("NumberOfBands", &nap::audio::WaveTableResource::mNumberOfBands, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::audio::Oscillator)
    RTTI_PROPERTY("Frequency", &nap::audio::Oscillator::mFrequency, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Amplitude", &nap::audio::Oscillator::mAmplitude, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("FmInput", &nap::audio::Oscillator::mFmInput, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("WaveTables", &nap::audio::Oscillator::mWaveTables, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("WaveTableSelection", &nap::audio::Oscillator::mWaveTableSelection, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::OscillatorInstance)
	RTTI_FUNCTION("getChannel", &nap::audio::OscillatorInstance::getChannel)
	RTTI_FUNCTION("getChannelCount", &nap::audio::OscillatorInstance::getChannelCount)
	RTTI_FUNCTION("getWaveTableCount", &nap::audio::OscillatorInstance::getWaveTableCount)
	RTTI_FUNCTION("selectWaveTable", &nap::audio::OscillatorInstance::selectWaveTable)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {

        bool WaveTableResource::init(utility::ErrorState& errorState)
        {
            mWave = mNodeManager.makeSafe<WaveTable>(mSize, mWaveform, mNumberOfBands);
            return true;
        }


		std::unique_ptr<AudioObjectInstance> Oscillator::createInstance(NodeManager& nodeManager, utility::ErrorState& errorState)
		{
			auto result = std::make_unique<OscillatorInstance>();
			if (!result->init(mChannelCount, nodeManager, errorState))
			{
				errorState.fail("Failed to initialize Oscillator");
				return nullptr;
			}

			for (auto& wave : mWaveTables)
				result->addWaveTable(wave->getWave());

			for (auto channel = 0; channel < mChannelCount; ++channel)
			{
				auto node = result->getChannel(channel);
				node->setFrequency(mFrequency[channel % mFrequency.size()]);
				node->setAmplitude(mAmplitude[channel % mAmplitude.size()]);
                if (mFmInput != nullptr)
                {
                    node->fmInput.connect(*mFmInput->getInstance()->getOutputForChannel(channel % mFmInput->getInstance()->getChannelCount()));
                }
			}

			if (mWaveTableSelection >= mWaveTables.size())
			{
				errorState.fail("Invalid waveform index for Oscillator: %s", mID.c_str());
				return nullptr;
			}
			result->selectWaveTable(mWaveTableSelection);

			return result;
		}


		bool OscillatorInstance::init(int channelCount, NodeManager& nodeManager, utility::ErrorState& errorState)
		{
			for (auto channel = 0; channel < channelCount; ++channel)
				mNodes.emplace_back(nodeManager.makeSafe<OscillatorNode>(nodeManager));

			return true;
		}


		void OscillatorInstance::selectWaveTable(int index)
		{
			assert(index < mWaveTables.size());
			for (auto& node : mNodes)
				node->setWave(mWaveTables[index]);
		}


        int OscillatorInstance::addWaveTable(const SafePtr<WaveTable> &waveTable)
        {
            mWaveTables.emplace_back(waveTable);
            return mWaveTables.size() - 1;
        }


    }
}
