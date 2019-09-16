#pragma once

// Audio includes
#include <audio/core/nodeobject.h>
#include <audio/node/circularbufferplayernode.h>

namespace nap
{

    namespace audio
    {

        class CircularBufferPlayer : public MultiChannel<CircularBufferPlayerNode> {
            RTTI_ENABLE(MultiChannelBase)
            
        public:
            CircularBufferPlayer() = default;
        };

    }

}
