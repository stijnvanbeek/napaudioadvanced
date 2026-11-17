/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Audio includes
#include <audio/core/nodeobject.h>
#include <audio/node/svfnode.h>
#include <nap/resourceptr.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Multichannel audio object to apply a state-variable filter to the input channels.
         */
        class NAPAPI SVF : public ParallelNodeObject<SVFNode>
        {
            RTTI_ENABLE(ParallelNodeObjectBase)
            
        public:
            SVF() = default;
            
            SVFNode::EMode mMode = SVFNode::EMode::LowPass;       ///< Property: 'Mode' Indicates whether the filter functions as lowpass, highpass, bandpass, notch or peak filter.
            std::vector<ControllerValue> mFrequency = { 440.f };  ///< Property: 'Frequency' Respectively the cutoff or center frequency of the filter, for each channel separately.
            std::vector<ControllerValue> mResonance = { 0.f };    ///< Property: 'Resonance' The resonance of the filter for each channel, 0 being the flat Butterworth response and 30 approaching self-oscillation.
            std::vector<ControllerValue> mGain = { 1.f };         ///< Property: 'Gain' Gain multiplier for each channel

        private:
            bool initNode(int channel, SVFNode& node, utility::ErrorState& errorState) override;
        };


        /**
         * Instance of SVF
         */
        using SVFInstance = ParallelNodeObjectInstance<SVFNode>;
        
    }
    
}

