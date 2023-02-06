/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <audio/core/nodeobject.h>
#include <audio/node/karplusstrongnode.h>

namespace nap
{

    namespace audio
    {

        /**
         * Object containing a @KarplusStrongNode on each channel.
         */
        using KarplusStrongObject = ParallelNodeObject<KarplusStrongNode>;
        using KarplusStrongInstance = ParallelNodeObjectInstance<KarplusStrongNode>;

    }

}