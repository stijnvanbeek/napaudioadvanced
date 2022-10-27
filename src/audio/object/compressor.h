#pragma once

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/multichannelwithinput.h>
#include <audio/core/nodeobject.h>
#include <audio/node/compressornode.h>

namespace nap
{

    namespace audio
    {

        using Compressor = MultiChannelWithInput<CompressorNode>;
        using CompressorInstance = ParallelNodeObjectInstance<CompressorNode>;

    }

}