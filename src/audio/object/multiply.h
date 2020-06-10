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

            std::vector<ResourcePtr<AudioObject>> mInputs; ///< property: Object generating the input signals to be multiplied.

        private:
            bool initNode(int channel, MultiplyNode& node, utility::ErrorState& errorState) override;
        };


    }

}
