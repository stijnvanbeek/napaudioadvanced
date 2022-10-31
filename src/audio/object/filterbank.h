#pragma once

#include <audio/core/multichannelwithinput.h>
#include <audio/node/filterbanknode.h>

namespace nap
{

    namespace audio
    {

        /**
         * Object containing a @FilterBankNode on each channel.
         */
        using FilterBankObject = MultiChannelWithInput<FilterBankNode>;
        using FilterBankInstance = ParallelNodeObjectInstance<FilterBankNode>;

    }

}