#pragma once

// Audio includes
#include <audio/core/nodeobject.h>
#include <audio/node/circularbufferplayernode.h>

namespace nap
{

    namespace audio
    {

        class NAPAPI CircularBufferPlayer : public ParallelNode<CircularBufferPlayerNode> {
            RTTI_ENABLE(ParallelNodeBase)
            
        public:
            CircularBufferPlayer() = default;
        };

    }

}
