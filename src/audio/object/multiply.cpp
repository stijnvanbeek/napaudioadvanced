/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "multiply.h"

RTTI_BEGIN_CLASS(nap::audio::Multiply)
    RTTI_PROPERTY("Inputs", &nap::audio::Multiply::mInputs, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::ParallelNodeObjectInstance<nap::audio::MultiplyNode>)
RTTI_END_CLASS

namespace nap
{

    namespace audio
    {

        bool Multiply::initNode(int channel, MultiplyNode& node, utility::ErrorState& errorState)
        {
            node.inputs.reserveInputs(mInputs.size());

            for (auto& input : mInputs)
                node.inputs.connect(*input->getInstance()->getOutputForChannel(channel % input->getInstance()->getChannelCount()));

            return true;
        }
    }

}
