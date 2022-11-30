/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Std includes
#include <atomic>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/utility/delay.h>
#include <audio/utility/linearsmoothedvalue.h>

namespace nap
{
    
    namespace audio
    {
    
        /**
         * Delay line with feedback and dry/wet control.
         */
        class NAPAPI DelayNode : public Node
        {
        public:
            /**
             * Constructor
             * @param manager NodeManager the node is processed on.
             * @param delayLineSize The size in samples of the delay line. Has to be a power of 2.
             */
            DelayNode(NodeManager& manager, int delayLineSize = 65536 * 8);
            
            InputPin input = { this }; /**< The audio input receiving the signal to be delayed. */
            OutputPin output = { this }; /**< The audio output with the processed signal. */
            
            /**
             * Sets the delay time in milliseconds. Ramp time specifies the time it takes to reach the new value.
             * @param value Delay time in ms
             * @param rampTime Time taken in ms to interpolate to the new delay time.
             */
            void setTime(TimeValue value, TimeValue rampTime = 0);
            
            /**
             * Sets the dry wet value.
             * @param value The dry/wet ratio. 0 means fully dry, 1. means fully wet.
             * @param rampTime The time it takes to reach the new value in ms.
             */
            void setDryWet(ControllerValue value, TimeValue rampTime = 0);
            
            /**
             * Specify the feedback amount of the delay line.
             * @pram value A multiplier for the feedback signal before its added to the input.
             */
            void setFeedback(ControllerValue value) { mFeedback = value; }
            
            /**
             * @return the current delay time.
             */
            int getTime() const { return mTime.getValue(); }
            
            /**
             * @return the dry/wet level. 0 means dry, 1. means fully wet.
             */
            ControllerValue getDryWet() const { return mDryWet.getValue(); }
            
            /**
             * @return the feedback amount
             */
            ControllerValue getFeedback() const { return mFeedback; }
            
        private:
            void process() override;
            
            Delay mDelay;
            LinearSmoothedValue<float> mTime = { 0, 44 }; // in samples
            LinearSmoothedValue<ControllerValue> mDryWet = { 0.5f, 44 };
            std::atomic<ControllerValue> mFeedback = { 0.f };
        };
        
    }
    
}
