#include "scalar.h"

RTTI_BEGIN_CLASS(nap::audio::Scalar)
        RTTI_PROPERTY("Scalar", &nap::audio::Scalar::mScalar, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("Input", &nap::audio::mScalar::mInput, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::ParallelNodeObjectInstance<nap::audio::ScalarNode>)
RTTI_END_CLASS

namespace nap
{

    namespace audio
    {

        bool Scalar::initNode(int channel, ScalarNode& node, utility::ErrorState& errorState)
        {
            if (mInput != nullptr)
                node.audioInput.connect(*mInput->getInstance()->getOutputForChannel(channel % mInput->getInstance()->getChannelCount()));
            node.setScalar(mScalar[channel % mScalr.size()], 0);

            return true;
        }


    }

}



