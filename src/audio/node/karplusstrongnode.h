#pragma once

#include <audio/utility/karplusstrong.h>
#include <audio/core/multichannelwithinput.h>

#include <audio/core/audionode.h>
#include <audio/core/nodeobject.h>

namespace nap
{

	namespace audio
	{

		class KarplusStrongNode : public Node
		{
		public:
			KarplusStrongNode(NodeManager& nodeManager) : Node(nodeManager)
			{
				reset(1000.f);
			}

			void reset(TimeValue maxDelayTime) { mKarplusStrong.reset(maxDelayTime * getNodeManager().getSamplesPerMillisecond()); }
			void setDelayTime(TimeValue time) { mKarplusStrong.setDelayTime(time * getNodeManager().getSamplesPerMillisecond(), getNodeManager().getSamplesPerMillisecond() * 5.f); }
			void setFeedback(ControllerValue value) { mKarplusStrong.setFeedback(value); }
			void setDamping(ControllerValue value) { mKarplusStrong.setDamping(value, getSampleRate()); }
			void setNegativePolarity(bool value) { mNegativePolarity = value; }

			InputPin audioInput = { this };
			OutputPin audioOutput = { this };

		private:
			void process() override
			{
				auto& outputBuffer = getOutputBuffer(audioOutput);
				auto inputBuffer = audioInput.pull();
				if (mNegativePolarity)
					for (auto i = 0; i < getBufferSize(); ++i)
						outputBuffer[i] = mKarplusStrong.processNegative((*inputBuffer)[i]);
				else
					for (auto i = 0; i < getBufferSize(); ++i)
						outputBuffer[i] = mKarplusStrong.processPositive((*inputBuffer)[i]);
			}

			bool mNegativePolarity = false;
			KarplusStrong<SampleValue> mKarplusStrong;
		};

	}

}
