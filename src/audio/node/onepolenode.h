/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Audio includes
#include <audio/utility/linearsmoothedvalue.h>
#include <audio/core/audionode.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * One pole lowpass filter
         */
        class NAPAPI OnePoleLowPassNode : public Node
        {
            RTTI_ENABLE(Node)
        public:
            OnePoleLowPassNode(NodeManager& nodeManager) : Node(nodeManager) { }
            
            InputPin input = { this };     ///< Audio input pin
            OutputPin output = { this };   ///< Audio output pin

            /**
             * Sets the cutoff frequency of the filter.
             * @param frequency Cutoff frequency in Hz
             */
            void setCutoffFrequency(ControllerValue frequency);

            /**
             * @return the cutoff frequency in Hz
             */
            ControllerValue getCutoffFrequency() const { return mCutOff; }

            /**
             * Sets the interpolation time of the cutoff frequency when calling setCutoffFrequency()
             * @param value The interpolation time in ms.
             */
            void setRampTime(TimeValue value);
            
        private:
            void process() override;
            LinearSmoothedValue<ControllerValue> a0 = { 0, 64 };
            LinearSmoothedValue<ControllerValue> b1 = { 0, 64 };
            ControllerValue mCutOff = 0.5;
            ControllerValue mTemp = 0.f;
        };
        

        /**
         * One pole highpass filter
         */
        class NAPAPI OnePoleHighPassNode : public Node
        {
            RTTI_ENABLE(Node)
        public:
            OnePoleHighPassNode(NodeManager& nodeManager) : Node(nodeManager) { }
            
            InputPin input = { this };     ///< Audio input pin
            OutputPin output = { this };   ///< Audio output pin

            /**
             * Sets the cutoff frequency of the filter.
             * @param frequency Cutoff frequency in Hz
             */
            void setCutoffFrequency(ControllerValue frequency);

            /**
             * @return the cutoff frequency in Hz
             */
            ControllerValue getCutoffFrequency() const { return mCutOff; }

            /**
             * Sets the interpolation time of the cutoff frequency when calling setCutoffFrequency()
             * @param value The interpolation time in ms.
             */
            void setRampTime(TimeValue value);
            
        private:
            void process() override;

            LinearSmoothedValue<ControllerValue> a0 = { 0, 64 };
            LinearSmoothedValue<ControllerValue> a1 = { 0, 64 };
            LinearSmoothedValue<ControllerValue> b1 = { 0, 64 };
            ControllerValue mCutOff = 0.5;
            ControllerValue mTemp1 = 0.f;
            ControllerValue mTemp2 = 0.f;
        };
        
    }
    
}
