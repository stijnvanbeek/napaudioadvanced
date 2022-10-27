#pragma once

#include <audio/utility/audiotypes.h>
#include <mathutils.h>
#include <audio/utility/vectorextension.h>

namespace nap
{

	namespace audio
	{

		template <typename real>
		class OnePoleLowPass
		{
		public:
			OnePoleLowPass() = default;

			real process(const real& input)
			{
				auto value = output + cf * (input - output);
				output = value;
				return output;
			}

			void setCutoffFrequency(float cutoffFrequency, float sampleRate)
			{
				real c = real(cutoffFrequency / sampleRate);
				cf = 1 - powVec(real(M_E), real(-math::PIX2) * c);
			}

		private:
			real cf = { 0.f };
			real output = { 0.f };
		};


		template <typename real>
		class OnePoleHighPass
		{
		public:
			OnePoleHighPass() = default;

			real process(const real& input)
			{
				output = a0 * input + a1 * previousInput + b1 * output;
				previousInput = input;
				return output;
			}

			void setCutoffFrequency(ControllerValue cutoffFrequency, float sampleRate)
			{
				real c = cutoffFrequency / sampleRate;
				real x = powVec(real(M_E), real(-M_2_PI) * c);
				real one = real(1.f);
				real two = real(2.f);
				a0 = (one + x) / two;
				a1 = -(one + x) / two;
				b1 = x;
			}

		private:
			real a0 = { 1.0 };
			real a1 = { 0.f };
			real b1 = { 0.f };
			real output = { 0.f };
			real previousInput = { 0.f };
		};

	}

}
