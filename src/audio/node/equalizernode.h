#pragma once

#include <audio/utility/equalizer.h>
#include <audio/core/audionode.h>
#include <audio/core/audionodemanager.h>

namespace nap
{
	namespace audio
	{

		/**
		 * 8 band equalizer with different modes for each band.
		 * Runs biquad filters for each band in series.
		 */
		class NAPAPI EqualizerNode : public Node
		{
			RTTI_ENABLE(Node)

		public:
			/**
			 * Number of bands
			 */
			static constexpr int bandCount = 8;

			/**
			 * Constructor
			 * @param nodeManager NodeManager the node runs on
			 */
			EqualizerNode(NodeManager& nodeManager);

			/**
			 * Prepare a band for processing with a set of parameters.
			 * @param index index of the band between 0 and bandCount
			 * @param mode mode of the band's filter
			 * @param frequency filter frequency in Hz
			 * @param gain band gain amplitude
			 * @param q band Q value (0.707 is neutral)
			 */
			void prepare(int index, Equalizer::Mode mode, ControllerValue frequency, ControllerValue gain, ControllerValue q);

			/**
			 * Modulate a band's properties while processing.
			 * @param index index of the band between 0 and bandCount
			 * @param mode mode of the band's filter
			 * @param frequency filter frequency in Hz
			 * @param gain band gain amplitude
			 * @param q band Q value (0.707 is neutral)
			 */
			void set(int index, Equalizer::Mode mode, ControllerValue frequency, ControllerValue gain, ControllerValue q);

			/**
			 * Enable or disable a specific band
			 * @param index index of the band between 0 and bandCount
			 * @param enabled true if enabled
			 */
			void setEnabled(int index, bool enabled) { mEnabled[index] = enabled; }

			InputPin mInput = { this };
			OutputPin mOutput = { this };

		private:
			void process() override;
			void sampleRateChanged(float sampleRate) override;

			std::array<Equalizer, bandCount> mEQs;
			std::array<std::atomic<bool>, bandCount> mEnabled = { false };
		};

	}
}
