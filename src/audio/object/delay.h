/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Audio includes
#include <audio/core/nodeobject.h>
#include <audio/node/delaynode.h>
#include <nap/resourceptr.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Multichannel audio object to apply a delay to the input channels.
         */
        class NAPAPI DelayObject : public ParallelNodeObject<DelayNode>
        {
            RTTI_ENABLE(ParallelNodeObjectBase)
            
        public:
            DelayObject() = default;
            
            std::vector<TimeValue> mTime = { 0.f };             ///< Property: 'Time' array of delay time values per output channel. If the size of the array is less than the number of channels it will be repeated.
            std::vector<ControllerValue> mFeedback = { 0.f };   ///< Property: 'Time' array of feedback values per output channel. If the size of the array is less than the number of channels it will be repeated.
            std::vector<TimeValue> mDryWet = { 0.f };           ///< Property: 'DryWet' array of dry wet balance levels per output channel. If the size of the array is less than the number of channels it will be repeated.
            ResourcePtr<AudioObject> mInput;                    ///< Property: "Input" AudioObject whose output channels will be used as inputs for the delay channels.
            
        private:
            bool initNode(int channel, DelayNode& node, utility::ErrorState& errorState) override;
        };


        /**
         * Instance of DelayObject
         */
        using DelayObjectInstance = ParallelNodeObjectInstance<DelayNode>;

    }
    
}

