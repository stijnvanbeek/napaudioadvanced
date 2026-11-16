/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Audio includes
#include <audio/utility/linearsmoothedvalue.h>

#include <audio/utility/vectorextension.h>

namespace nap
{
    namespace audio
    {

        /**
         * Helper object to calculate a multiple of 4 or 8 biquad filters simultaneously with SSE or AVX vector extensions using @float4 or @float8.
         *
         * The coefficients follow the textbook convention for a second order IIR filter:
         *
         *          b0 + b1 * z^-1 + b2 * z^-2
         *   H(z) = --------------------------
         *          1  + a1 * z^-1 + a2 * z^-2
         *
         * which corresponds to the difference equation:
         *
         *   y[n] = b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] - a1 * y[n-1] - a2 * y[n-2]
         *
         * The denominator is normalized so that a0 == 1, which is why a0 is not stored. The feedback
         * coefficients a1 and a2 are subtracted, matching the minus signs in the equation above.
         *
         * c0 and d0 are the wet and dry gains: the filter output is c0 * (filtered signal) + d0 * (input),
         * so c0 = 1, d0 = 0 gives a fully wet filter and c0 = 1, d0 = 1 adds the dry signal on top.
         *
         * @tparam real Should be float, @float4 or @float8.
         */
        template <typename real>
        class NAPAPI BiquadFilter
        {
        public:
            BiquadFilter() : b0(real(0), 64), b1(real(0), 64), b2(real(0), 64), a1(real(0), 64), a2(real(0), 64), c0(real(0), 64), d0(real(0), 64), h1(0), h2(0) { }

            // Delete copy and move constructor
            BiquadFilter(const BiquadFilter&) = delete;
            BiquadFilter& operator=(const BiquadFilter&) = delete;

            /**
             * Sets the coefficient of all the filters.
             * @param _b0 Numerator coefficient for the current input sample.
             * @param _b1 Numerator coefficient for the input sample one step back.
             * @param _b2 Numerator coefficient for the input sample two steps back.
             * @param _a1 Denominator coefficient for the output sample one step back.
             * @param _a2 Denominator coefficient for the output sample two steps back.
             * @param _c0 Wet gain, applied to the filtered signal.
             * @param _d0 Dry gain, applied to the unfiltered input signal.
             */
            void setCoefficients(real _b0, real _b1, real _b2, real _a1, real _a2, real _c0, real _d0)
            {
                b0.setValue(_b0);
                b1.setValue(_b1);
                b2.setValue(_b2);
                a1.setValue(_a1);
                a2.setValue(_a2);
                c0.setValue(_c0);
                d0.setValue(_d0);
            }

            /**
             * Sets the coefficient of all the filters immediately.
             * @param _b0 Numerator coefficient for the current input sample.
             * @param _b1 Numerator coefficient for the input sample one step back.
             * @param _b2 Numerator coefficient for the input sample two steps back.
             * @param _a1 Denominator coefficient for the output sample one step back.
             * @param _a2 Denominator coefficient for the output sample two steps back.
             * @param _c0 Wet gain, applied to the filtered signal.
             * @param _d0 Dry gain, applied to the unfiltered input signal.
             */
            void resetCoefficients(real _b0, real _b1, real _b2, real _a1, real _a2, real _c0, real _d0)
            {
                b0.reset(_b0);
                b1.reset(_b1);
                b2.reset(_b2);
                a1.reset(_a1);
                a2.reset(_a2);
                c0.reset(_c0);
                d0.reset(_d0);
            }

            /**
             * Process one input sample for all the filters simultaneously.
             * @param input sample value
             * @return output sample of the filter
             */
            real process(const real value)
            {
                real result = value * b0.getNextValue() + h1;

                h1 = value * b1.getNextValue() + h2 - a1.getNextValue() * result;
                h2 = value * b2.getNextValue()      - a2.getNextValue() * result;

                return result * c0.getNextValue() + value * d0.getNextValue();
            }

            /**
             * Sets the smoothing time in samples
             * @param count Number of samples
             */
            void setSmoothing(int count)
            {
                b0.setStepCount(count);
                b1.setStepCount(count);
                b2.setStepCount(count);
                a1.setStepCount(count);
                a2.setStepCount(count);
                c0.setStepCount(count);
                d0.setStepCount(count);
            }

        private:
            LinearSmoothedValue<real> b0; // Numerator, current input sample
            LinearSmoothedValue<real> b1; // Numerator, one step back
            LinearSmoothedValue<real> b2; // Numerator, two steps back
            LinearSmoothedValue<real> a1; // Denominator, one step back
            LinearSmoothedValue<real> a2; // Denominator, two steps back
            LinearSmoothedValue<real> c0; // Wet gain
            LinearSmoothedValue<real> d0; // Dry gain
            real h1;
            real h2;
        };

    }
}
