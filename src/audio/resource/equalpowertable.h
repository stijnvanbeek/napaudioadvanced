/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Nap includes
#include <nap/resource.h>

// Audio includes
#include <audio/utility/translator.h>
#include <audio/utility/safeptr.h>
#include <audio/service/audioservice.h>

namespace nap
{

    // Forward declarations
    class Core;

    namespace audio
    {

        /**
         * Resource managing an equal power lookup table that can be shared across multiple audio processes to save memory usage.
         */
        class NAPAPI EqualPowerTable : public Resource
        {
            RTTI_ENABLE(Resource)

        public:
            EqualPowerTable(Core& core);

            // Inherited from Resource
            bool init(utility::ErrorState& errorState) override;

            /**
             * Looks up an input value in the equal power table and outputs the lookup value.
             * @param inputValue The value to translate
             * @return The corresponding value found in the equal power table
             */
            float translate(const float& inputValue) { return mTable->translate(inputValue); }

            /**
             * @return Pointer to the lookup table object
             */
            audio::SafePtr<EqualPowerTranslator> getTable() { return mTable.get(); }

            int mSize = 256; // Property: 'Size' Size of the table in floats.

        private:
            NodeManager* mNodeManager = nullptr;
            audio::SafeOwner<EqualPowerTranslator> mTable = nullptr;
        };

    }

}



