#pragma once

// Audio includes
#include <audio/core/nodeobject.h>
#include <audio/node/scalarnode.h>
#include <nap/resourceptr.h>

namespace nap
{

    namespace audio
    {

        /**
        * Multichannel audio object to apply a gain to the input channels.
        */
        class NAPAPI Scalar : public ParallelNodeObject<ScalarNode>
        {
        RTTI_ENABLE(ParallelNodeObjectBase)

        public:
            Scalar() = default;

            std::vector<ControllerValue> mScalar = { 1.f }; ///< property: 'Scalar' array of scalar values per output channel. If the size of the array is less than the number of channels it will be repeated.
            ResourcePtr<AudioObject> mInput = nullptr; ///< property: Object generating the input signal of the scalar.

        private:
            bool initNode(int channel, ScalarNode& node, utility::ErrorState& errorState) override;
        };



    }

}