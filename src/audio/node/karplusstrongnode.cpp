/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "karplusstrongnode.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::KarplusStrongNode)
		RTTI_PROPERTY("input", &nap::audio::KarplusStrongNode::audioInput, nap::rtti::EPropertyMetaData::Embedded)
		RTTI_PROPERTY("output", &nap::audio::KarplusStrongNode::audioOutput, nap::rtti::EPropertyMetaData::Embedded)
		RTTI_FUNCTION("setDelayTime", &nap::audio::KarplusStrongNode::setDelayTime)
		RTTI_FUNCTION("setFeedback", &nap::audio::KarplusStrongNode::setFeedback)
		RTTI_FUNCTION("setDamping", &nap::audio::KarplusStrongNode::setDamping)
		RTTI_FUNCTION("setNegativePolarity", &nap::audio::KarplusStrongNode::setNegativePolarity)
RTTI_END_CLASS

namespace nap
{

    namespace audio
    {

        void KarplusStrongNode::process()
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

    }

}

