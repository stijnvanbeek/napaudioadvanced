/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Audio includes
#include <audio/core/nodeobject.h>
#include <audio/node/circularbufferplayernode.h>

namespace nap
{

    namespace audio
    {

        /**
         * Object to play back audio from a circular buffer
         */
        class NAPAPI CircularBufferPlayer : public ParallelNodeObject<CircularBufferPlayerNode> {
            RTTI_ENABLE(ParallelNodeObjectBase)
            
        public:
            CircularBufferPlayer() = default;
        };

    }

}
