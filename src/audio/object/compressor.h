/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/nodeobject.h>
#include <audio/core/nodeobject.h>
#include <audio/node/compressornode.h>

namespace nap
{

    namespace audio
    {

        /**
         * Multichannel compressor audio object
         */
        using Compressor = ParallelNodeObject<CompressorNode>;

        /**
         * Instance of Compressor
         */
        using CompressorInstance = ParallelNodeObjectInstance<CompressorNode>;

    }

}