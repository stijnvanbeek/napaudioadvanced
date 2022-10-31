#pragma once

#include <audio/core/multichannelwithinput.h>
#include <audio/node/karplusstrongnode.h>

namespace nap
{

    namespace audio
    {

        /**
         * Object containing a @KarplusStrongNode on each channel.
         */
        using KarplusStrongObject = MultiChannelWithInput<KarplusStrongNode>;
        using KarplusStrongInstance = ParallelNodeObjectInstance<KarplusStrongNode>;

    }

}