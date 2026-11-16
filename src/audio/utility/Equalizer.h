#pragma once

#include <audio/utility/biquad.h>
#include <audio/utility/audiotypes.h>

namespace nap
{

	namespace audio
	{

		/**
		 * Single band equalizer, implemented as a biquad filter.
		 * Supports a peaking band and low / high shelves.
		 */
		class NAPAPI Equalizer
		{
		public:
			enum class Mode { Peak, LowShelf, HighShelf, LowPass, HighPass };

			Equalizer() = default;

			/**
			 * Prepare for processing with a set of parameters.
			 * @param index index of the band between 0 and bandCount
			 * @param mode mode of the band's filter
			 * @param frequency filter frequency in Hz
			 * @param gain band gain amplitude
			 * @param q band Q value (0.707 is neutral)
			 */
			void prepare(Mode mode, ControllerValue frequency, float sampleRate, ControllerValue gain, ControllerValue q);

			/**
			 * Modulate properties while processing. Uses smoothing.
			 * @param index index of the band between 0 and bandCount
			 * @param mode mode of the band's filter
			 * @param frequency filter frequency in Hz
			 * @param gain band gain amplitude
			 * @param q band Q value (0.707 is neutral)
			 */
			void set(Mode mode, ControllerValue frequency, float sampleRate, ControllerValue gain, ControllerValue q);

			/**
			 * Sets the smooth time used for modulating.
			 * @param samples time in samples.
			 */
			void setSmoothing(int samples) { mBiquad.setSmoothing(samples); }

			/**
			 * Sets the samplerate
			 * @param sampleRate in Hz
			 */
			void setSampleRate(float sampleRate);

			SampleValue process(SampleValue input) { return mBiquad.process(input); }

			/**
			 * Calculates the biquad coefficients for a single band. Peak and shelf modes use the RBJ
			 * audio EQ cookbook, the lowpass and highpass modes use the second order Butterworth
			 * algorithm also used by FilterNode, generalized with a resonance Q: Q = 1/sqrt(2) is the
			 * maximally flat Butterworth response, higher Q adds a resonant peak at the cutoff.
			 * Static because the coefficients only depend on the band settings and the sample rate,
			 * not on any filter state, so they can be computed without an Eq instance.
			 *
			 * The denominator is normalized so that a0 == 1, which is why it is not part of the
			 * signature.
			 * @param mode filter mode of the band
			 * @param frequency center frequency in Hz
			 * @param gain boost or cut in dB
			 * @param q quality factor
			 * @param sampleRate sample rate in Hz
			 * @param b0 numerator coefficient for the current input sample
			 * @param b1 numerator coefficient for the input sample one step back
			 * @param b2 numerator coefficient for the input sample two steps back
			 * @param a1 denominator coefficient for the output sample one step back
			 * @param a2 denominator coefficient for the output sample two steps back
			 * @param c0 wet gain, applied to the filtered signal
			 * @param d0 dry gain, applied to the unfiltered input signal
			 */
			static void calcCoeffs(Mode mode, ControllerValue frequency, ControllerValue gain, ControllerValue q,
				float sampleRate, float& b0, float& b1, float& b2, float& a1, float& a2, float& c0, float& d0);

		private:
			BiquadFilter<float> mBiquad;
			Mode mMode = Mode::Peak;
			ControllerValue mFrequency = 220.f;
			ControllerValue mGain = 0.f;
			ControllerValue mQ = 0.f;
			ControllerValue mSampleRate = 44100.f;
		};

	}

}
