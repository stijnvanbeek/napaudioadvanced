/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <audio/utility/karplusstrong.h>
#include <audio/core/audionode.h>
#include <audio/core/audionodemanager.h>

namespace nap
{

	namespace audio
	{

	    /**
	     * Node that performs the Karplus Strong algorithm:
	     * a comb filter (or tuned feedback loop) with a onepole lowpass filter within th feedback loop.
	     * Wraps the KarplusStrong class in a Node.
	     */
		class KarplusStrongNode : public Node
		{
		public:
			KarplusStrongNode(NodeManager& nodeManager) : Node(nodeManager)
			{
				reset(1000.f);
			}

			/**
			 * Reset the filter and zeros the buffers.
			 * @param maxDelayTime Maximum supported delay tme in ms. This will be converted to a power of 2 in samples.
			 */
			void reset(TimeValue maxDelayTime) { mKarplusStrong.reset(maxDelayTime * getNodeManager().getSamplesPerMillisecond()); }

			/**
			 * Set the delay time in ms.
			 * @param time Delay time in ms.
			 */
			void setDelayTime(TimeValue time) { mKarplusStrong.setDelayTime(time * getNodeManager().getSamplesPerMillisecond(), getNodeManager().getSamplesPerMillisecond() * 5.f); }

			/**
			 * Sets the feedback amount.
			 * @param value The feedback amount is specified as the gain multiplier of the feedback signal.
			 */
			void setFeedback(ControllerValue value) { mKarplusStrong.setFeedback(value); }

			/**
			 * Sets the damping value, which equals the cutoff frequency of the lowpass filter in the feedback loop.
			 * @param value Cutoff f
			 */
			void setDamping(ControllerValue value) { mKarplusStrong.setDamping(value, getSampleRate()); }

			/**
			 * Sets the polarity of the feedback loop. A negative polarity means that the feedback signal will be subtracted from the input instead of added.
			 * @param value True for negative polarity, false for a positive polarity.
			 */
			void setNegativePolarity(bool value) { mNegativePolarity = value; }

			InputPin audioInput = { this };       ///< Connect the input signal to this pin.
			OutputPin audioOutput = { this };     ///< Connect this pin to another node's input

		private:
			void process() override;

			bool mNegativePolarity = false;
			KarplusStrong<SampleValue> mKarplusStrong;
		};

	}

}
