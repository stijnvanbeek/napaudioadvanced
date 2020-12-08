#include <audio/node/oscillatornode.h>

#include <math.h>

// Rtti includes
#include <rtti/rtti.h>

// Audio includes
#include <audio/utility/audiofunctions.h>
#include <audio/utility/safeptr.h>
#include <audio/core/audionodemanager.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::OscillatorNode)
    RTTI_FUNCTION("setFrequency", &nap::audio::OscillatorNode::setFrequency)
    RTTI_FUNCTION("setAmplitude", &nap::audio::OscillatorNode::setAmplitude)
    RTTI_FUNCTION("setPhaseOffset", &nap::audio::OscillatorNode::setPhase)
    RTTI_PROPERTY("fmInput", &nap::audio::OscillatorNode::fmInput, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("audioOutput", &nap::audio::OscillatorNode::output, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
// --- Wavetable --- //

		WaveTable::WaveTable(long size, Waveform waveform, int numberOfBands)
        {
			if (waveform == Waveform::Sine)
			{
				mData.resize(1, size);
				auto& bandData = mData[0];
				mBandWidth = Nyquist;
				auto step = M_PI * 2 / size;
				for (int i = 0; i < size; i++)
					bandData[i] = sin(i * step);
			}
			else
			{
				mData.resize(numberOfBands, size);
				mBandWidth = Nyquist / numberOfBands;
				for (auto band = 0; band < numberOfBands; ++band)
				{
					auto bandFrequency = mBandWidth * (band + 1);
					auto& bandData = mData[band];

					auto harmonic = 1;
					while (bandFrequency * harmonic < Nyquist)
					{
						auto step = M_PI * 2 / size;
						auto a = harmonic > 1.f ? 1.f / (harmonic - 1.f) : 1.f;
						for (auto i = 0; i < size; i++)
							bandData[i] += a * sin(i * step * harmonic);
						harmonic++;
					}

				}
			}
        }


        void WaveTable::normalize()
        {
			for (auto band = 0; band < mData.getChannelCount(); band++)
			{
				auto& bandData = mData[band];
				SampleValue max, min;
				max = min = bandData[0];

				for(int i = 0; i < bandData.size(); i++){
					if(bandData[i] > max) max = bandData[i];
					if(bandData[i] < min) min = bandData[i];
				}

				if(max < -min) max = -min;
				for(int i = 0; i < bandData.size(); i++)
					bandData[i] /= max;
			}
        }

        
        SampleValue WaveTable::interpolate(double index, float frequency) const
        {
            int floor = index;
            SampleValue frac = index - floor;

			auto band = int(frequency / mBandWidth);
			auto& data = mData[band];
            
            auto v1 = data[wrap(floor, data.size())];
            auto v2 = data[wrap(floor + 1, data.size())];

            return lerp(v1, v2, frac);
        }

        
// --- Oscillator --- //


        OscillatorNode::OscillatorNode(NodeManager& manager) : Node(manager)
        {
            mFrequency.setStepCount(getNodeManager().getSamplesPerMillisecond());
            mAmplitude.setStepCount(getNodeManager().getSamplesPerMillisecond());
        }


        OscillatorNode::OscillatorNode(NodeManager& manager, SafePtr<WaveTable> wave) :
            Node(manager), mWave(wave)
        {
            mStep = mWave->getSize() / getNodeManager().getSampleRate();
            mFrequency.setStepCount(getNodeManager().getSamplesPerMillisecond());
            mAmplitude.setStepCount(getNodeManager().getSamplesPerMillisecond());
        }

        
        void OscillatorNode::process()
        {
            auto& outputBuffer = getOutputBuffer(output);
            SampleBuffer* fmInputBuffer = fmInput.pull();

            if (mWave == nullptr)
            {
                for (auto i = 0; i < getBufferSize(); ++i)
                    outputBuffer[i] = 0.f;
                return;
            }

            auto waveSize = mWave->getSize();
            auto step = mStep.load();
            auto phaseOffset = mPhaseOffset.load();
            
            for (auto i = 0; i < getBufferSize(); i++)
            {
				auto frequency = mFrequency.getNextValue();

				// calculate new value, use wave as a lookup table
                auto val = mAmplitude.getNextValue() * mWave->interpolate(mPhase + phaseOffset, frequency);

				// calculate new phase
				if (fmInputBuffer)
                    mPhase += ((*fmInputBuffer)[i] + 1) * frequency * step;
                else
                    mPhase += frequency * step;
                if (mPhase > waveSize)
                    mPhase -= waveSize;
                
                outputBuffer[i] = val;
            }
        }

        
        void OscillatorNode::setAmplitude(ControllerValue amplitude, TimeValue rampTime)
        {
            mAmplitude.setValue(amplitude);
        }
        
        
        void OscillatorNode::setPhase(ControllerValue phase)
        {
            mPhaseOffset = phase * mWave->getSize();
        }


        void OscillatorNode::setFrequency(SampleValue frequency, TimeValue rampTime)
        {
            mFrequency.setValue(frequency);
        }
        
        
        void OscillatorNode::setWave(SafePtr<WaveTable> wave)
        {
            mWave = wave;
            mStep = mWave->getSize() / getNodeManager().getSampleRate();
        }
        
        
        void OscillatorNode::sampleRateChanged(float sampleRate)
        {
            mStep = mWave->getSize() / sampleRate;
        }
    }
}
