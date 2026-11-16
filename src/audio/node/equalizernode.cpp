#include "equalizernode.h"

namespace nap
{

	namespace audio
	{

		EqualizerNode::EqualizerNode(NodeManager &nodeManager): Node(nodeManager)
		{
			for (auto& eq : mEQs)
				eq.prepare(Equalizer::Mode::Peak, 440, nodeManager.getSampleRate(), 0.f, 0.f);

			for (auto& enable : mEnabled)
				enable.store(false);
		}


		void EqualizerNode::prepare(int index, Equalizer::Mode mode, ControllerValue frequency, ControllerValue gain, ControllerValue q)
		{
			mEQs[index].prepare(mode, frequency, getNodeManager().getSampleRate(), gain, q);
		}


		void EqualizerNode::set(int index, Equalizer::Mode mode, ControllerValue frequency, ControllerValue gain, ControllerValue q)
		{
			mEQs[index].set(mode, frequency, getNodeManager().getSampleRate(), gain, q);
		}


		void EqualizerNode::process()
		{
			auto& outputBuffer = getOutputBuffer(mOutput);
			auto inputBuffer = mInput.pull();

			if (inputBuffer == nullptr)
			{
				std::fill(outputBuffer.begin(), outputBuffer.end(), 0.f);
				return;
			}

			int sampleIndex = 0;
			for (auto& sample : outputBuffer)
			{
				sample = (*inputBuffer)[sampleIndex++];
				int eqIndex = 0;
				for (auto& eq : mEQs)
					if (mEnabled[eqIndex++])
						sample = eq.process(sample);
			}
		}


		void EqualizerNode::sampleRateChanged(float sampleRate)
		{
			for (auto& eq : mEQs)
				eq.setSampleRate(sampleRate);
		}
	}

}