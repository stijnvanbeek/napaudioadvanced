/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Nap includes
#include <nap/signalslot.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Used to smooth changed to a value linearly over time, using a fixed smoothing time.
         * A slightly optimized version to the LinearSmoothedValue in modnapaudio.
         * Pitfall is that the update() method has to be called manually each audio callback.
         */
        template <typename T>
        class FastLinearSmoothedValue
        {

        public:
            /**
             * Constructor
             * @param initValue Initial value of the value
             * @param stepCount The number of steps in samples the value takes to reach a new destination
             */
            FastLinearSmoothedValue(const T& initValue, int stepCount) : mNewDestination(initValue), mValue(initValue), mDestination(initValue)
            {
                mStepCount = stepCount;
            }
            
            /**
             * Change the number of steps the value takes to reach a new destination.
             * @param stepCount Number of steps in samples
             */
            void setStepCount(int stepCount)
            {
                mStepCount = stepCount;
            }
            
            /**
             * Start a ramp
             * @param destination The destination value of the next ramp
             */
            void setValue(const T& destination)
            {
                mNewDestination = destination;
            }
			
			
			/**
			 * Resets value without ramping.
			 * @param value The value.
			 */
			void reset(const T& value)
			{
				mValue = value;
				mNewDestination = value;
				mDestination = value;
				mIncrement = 0;
				mStepCounter = 0;
			}

			
            /**
             * Should be called each audio callback in order to update the status.
             * This way the (mNewDestination != mDestination) will be performed per buffer instead of per sample, and therefore performs a little more efficient than the regular LinearSmoothedValue
             */
            void update()
            {
                if (mNewDestination != mDestination)
                {
                    mDestination = mNewDestination;
                    mStepCounter = mStepCount;
                    if (mStepCounter == 0)
                        mValue = mDestination;
                    else
                        mIncrement = (mDestination - mValue) / T(mStepCount);
                }
            }
            
            /**
             * Take the next step in the current ramp and return the updated value.
             * Should only be called from the audio thread.
             * @return The updated value
             */
            T getNextValue()
            {
                if (mStepCounter > 0)
                {
                    mValue = mValue + mIncrement;
                    mStepCounter--;
                    if (mStepCounter == 0)
                        mValue = mDestination;
                }
                
                return mValue;
            }
            
            /**
             * Should only be called from the audio thread
             * @return The current value
             */
            inline T getValue() const
            {
                return mValue;
            }

            /**
             * @return The destination of the current ramp. If the destination has been reached this value equals the current value.
             */
            inline T getDestination() const { return mNewDestination; }
            
            /**
             * Should only be called from the audio thread.
             * @return True when currently playing a ramp.
             */
            inline bool isRamping() const { return mStepCounter > 0 || mDestination != mNewDestination; }
            
        private:
            std::atomic<T> mNewDestination = 0;
            
            T mValue; // Value that is being controlled by this object.
            T mIncrement; // Increment value per step of the current ramp when mode is linear.
            T mDestination = 0; // Destination value of the current ramp.
            std::atomic<int> mStepCount = 0; // Number of steps in the ramp.
            int mStepCounter = 0; // Current step index, 0 means at destination
        };
        
        
    }
    
}

