/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "equalpowertable.h"

// Nap includes
#include <nap/core.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::EqualPowerTable)
        RTTI_CONSTRUCTOR(nap::Core&)
        RTTI_PROPERTY("Size", &nap::audio::EqualPowerTable::mSize, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{

    namespace audio
    {

        EqualPowerTable::EqualPowerTable(Core &core) : Resource()
        {
            auto audioService = core.getService<AudioService>();
            assert(audioService != nullptr);
            mNodeManager = &audioService->getNodeManager();
        }


        bool EqualPowerTable::init(utility::ErrorState& errorState)
        {
            if (mSize <= 0)
            {
                errorState.fail("Size must be greater than zero.");
                return false;
            }

            mTable = mNodeManager->makeSafe<EqualPowerTranslator>(mSize);
            return true;
        }

    }

}
