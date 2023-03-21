/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Audio includes
#include <audio/core/nodeobject.h>
#include <audio/node/filternode.h>
#include <nap/resourceptr.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Multichannel audio object to apply a filter to the input channels.
         */
        class NAPAPI Filter : public ParallelNodeObject<FilterNode>
        {
            RTTI_ENABLE(ParallelNodeObjectBase)
            
        public:
            Filter() = default;
            
            FilterNode::EMode mMode = FilterNode::EMode::LowPass; ///< Property: 'Mode' Indicates of the filter functions as lowpass, highpass, bandpass, resonating lowpass or resonating highpass.
            std::vector<ControllerValue> mFrequency = { 440.f };  ///< Property: 'Frequency' Respectively the cutoff or center frequency of the filter, for each band separately.
            std::vector<ControllerValue> mResonance = { 0.f };    ///< Property: 'Resonance' In the case of a resonant filter the resonance around the cutoff frequency for each band.
            std::vector<ControllerValue> mBand = { 100.f };       ///< Property: 'Band' Bandwidth in Hz for each band in case of a bandpass filter
            std::vector<ControllerValue> mGain = { 1.f };         ///< Property: 'Gain' Gain multiplier for each band
            ResourcePtr<AudioObject> mInput = nullptr;            ///< property: "Input" @AudioObject whose output channels will be used as inputs for the filter channels.
            
        private:
            bool initNode(int channel, FilterNode& node, utility::ErrorState& errorState) override;
        };


        /**
         * Instance of Filter
         */
        using FilterInstance = ParallelNodeObjectInstance<FilterNode>;
        
    }
    
}

