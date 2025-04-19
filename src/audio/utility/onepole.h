/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <audio/utility/audiotypes.h>
#include <mathutils.h>
#include <audio/utility/vectorextension.h>

#include <atomic>

namespace nap
{

	namespace audio
	{

	    /**
	     * One pole lowpass filter algorithm
	     * @tparam real Can be float, float4 or float8 to enable SIMD processing
	     */
		template <typename real>
		class OnePoleLowPass
		{
		public:
			OnePoleLowPass() = default;

			/**
			 * Process a single input smaple
			 * @param input Input value to process by the filter
			 * @return Output sample value
			 */
			real process(const real& input)
			{
				auto value = output + cf * (input - output);
				output = value;
				return output;
			}

            /**
             * Set the cutoff frequency
             * @param cutoffFrequency The new cutoff frequency in HZ
             * @param sampleRate The samplerate the filter runs on
             */
			void setCutoffFrequency(float cutoffFrequency, float sampleRate)
			{
				real c = real(cutoffFrequency / sampleRate);
				cf = 1 - powVec(real(math::E), real(-math::PIX2) * c);
			}

		private:
			std::atomic<real> cf = { 0.f };
			real output = { 0.f };
		};


        /**
         * One pole high pass filter algorithm
         * @tparam real Can be float, float4 or float8 to enable SIMD processing
         */
		template <typename real>
		class OnePoleHighPass
		{
		public:
			OnePoleHighPass() = default;

            /**
             * Process a single input smaple
             * @param input Input value to process by the filter
             * @return Output sample value
             */
			real process(const real& input)
			{
				output = a0 * input + a1 * previousInput + b1 * output;
				previousInput = input;
				return output;
			}

			/**
			 * Set the cutoff frequency
			 * @param cutoffFrequency The new cutoff frequency in HZ
			 * @param sampleRate The samplerate the filter runs on
			 */
			void setCutoffFrequency(ControllerValue cutoffFrequency, float sampleRate)
			{
				real c = cutoffFrequency / sampleRate;
				real x = powVec(real(math::E), real(-math::M2_PI) * c);
				real one = real(1.f);
				real two = real(2.f);
				a0 = (one + x) / two;
				a1 = -(one + x) / two;
				b1 = x;
			}

		private:
			std::atomic<real> a0 = { 1.0 };
			std::atomic<real> a1 = { 0.f };
			std::atomic<real> b1 = { 0.f };
			real output = { 0.f };
			real previousInput = { 0.f };
		};

	}

}
