/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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

        /**
         * Instance of FilterBankObject
         */
        using FilterBankInstance = ParallelNodeObjectInstance<FilterBankNode>;

    }

}