/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/nodeobject.h>
#include <audio/node/equalizernode.h>

namespace nap
{

    namespace audio
    {

        /**
         * Multichannel equalizer audio object
         */
        using EqualizerObject = ParallelNodeObject<EqualizerNode>;

        /**
         * Instance of EqualizerObject
         */
        using EqualizerObjectInstance = ParallelNodeObjectInstance<EqualizerNode>;

    }

}
