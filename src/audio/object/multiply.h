/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Audio includes
#include <audio/core/nodeobject.h>
#include <audio/node/multiplynode.h>
#include <nap/resourceptr.h>

namespace nap
{

    namespace audio
    {

        /**
         * Multichannel audio object to multiply a number of inputs.
         */
        class NAPAPI Multiply : public ParallelNodeObject<MultiplyNode>
        {
        RTTI_ENABLE(ParallelNodeObjectBase)

        public:
            Multiply() = default;

            std::vector<ResourcePtr<AudioObject>> mInputs; ///< Property: 'Inputs' Object generating the input signals to be multiplied.

        private:
            bool initNode(int channel, MultiplyNode& node, utility::ErrorState& errorState) override;
        };


        /**
         * Instance of Multiply
         */
        using MultiplyInstance = ParallelNodeObjectInstance<MultiplyNode>;

    }

}
