#pragma once

// Audio includes
#include <audio/core/nodeobject.h>
#include <audio/node/circularbufferplayernode.h>

namespace nap
{
    
    namespace audio
    {

        using CircularBufferPlayer = MultiChannel<CircularBufferPlayerNode>;

    }
    
}

