/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Std includes
#include <atomic>

// Nap includes
#include <nap/resourceptr.h>
#include <utility/threading.h>

// Audio includes
#include <audio/utility/biquad.h>
#include <audio/utility/onepole.h>

#include <audio/core/audionode.h>
#include <audio/utility/dirtyflag.h>

namespace nap
{
    
    namespace audio
    {

		/**
		 * Processes a maximum of 8 parallel bandpass filters on the input signal using AVX2 optimization.
		 */
		class NAPAPI FilterBank
		{
		public:
			FilterBank() = default;
			~FilterBank();

			/**
			 * Sets the number of filters being processed. The maximum is 8.
			 * @param count Number of filters being processed in parallel.
			 */
			void setFilterCount(unsigned int count);

			/**
			 * @return: The number of filters being processed
			 */
			int getFilterCount() const { return mFilterCount.load(); }

			/**
			 * Sets the parameters of all filters to the values within the vector arguments. If the sizes of the vectors are shorter than 8, the content will be repeated.
			 * @param centerFrequency Centerfrequency in Hz for each of the filters
			 * @param bandWidth Bandwidths in Hz for each of the filters
			 * @param gain Gain multiplier for each of the filters
			 * @param sampleRate Samplerate the filterbank will be processed at.
			 */
			void setParameters(const std::vector<ControllerValue>& centerFrequency, const std::vector<ControllerValue>& bandWidth, const std::vector<ControllerValue>& gain, float sampleRate);

			/**
			 * Set the low shelf parallel lowpass filter. This can be used to add the low end that falls outside the bandpass filter bands. Useful if the filterbank is used as EQ.
			 * @param cutoffFrequency Cutoff frequency of the low shelf lowpass filter. (normally below the lowest band)
			 * @param gain Gain of the low shelf band
			 * @param sampleRate The sample rate the system is running at.
			 */
			void setLowShelf(ControllerValue cutoffFrequency, ControllerValue gain, float sampleRate);

			/**
			 * Process the samples in inputBuffer and store the output in outputBuffer. Both parameters are allowed to be the same buffer for replacement processing.
			 * @param inputBuffer Buffer with input samples. Has to be at least as big as the outputBuffer.
			 * @param outputBugger BUffer containing the output samples after execution.
			 */
			void processBuffer(SampleBuffer& inputBuffer, SampleBuffer& outputBuffer);

		private:
			std::atomic<int> mFilterCount = { 1 };
			BiquadFilter<float8> mFilter;
            OnePoleLowPass<SampleValue> mLowShelf;
			std::atomic<ControllerValue> mLowShelfGain = 0.f;

			using UpdateFunction = std::function<void()>;
			std::atomic<UpdateFunction*> mUpdateFunction = { nullptr };
			moodycamel::ReaderWriterQueue<UpdateFunction*> mDeletionQueue;
		};
     
        /**
         * Node wrapper for FilterBank
         */
        class NAPAPI FilterBankNode : public Node
        {
            RTTI_ENABLE(Node)
            
        public:
            FilterBankNode(NodeManager& manager) : Node(manager) { }

            InputPin audioInput = { this }; /**< The audio input receiving the signal to be processed. */
            OutputPin output = { this }; /**< The audio output with the processed signal. */
            
            /**
             * Sets the number of filters being processed. The maximum is 8.
             * @param count Number of filters being processed in parallel.
             */
            void setFilterCount(unsigned int count) { mFilterBank.setFilterCount(count); }
            
            /**
             * @return: The number of filters being processed
             */
            int getFilterCount() const { return mFilterBank.getFilterCount(); }
            
            /**
             * Sets the parameters of all filters to the values within the vector arguments. If the sizes of the vectors are shorter than 8, the content will be repeated.
             * @param centerFrequency Centerfrequency in Hz for each of the filters
             * @param bandWidth Bandwidths in Hz for each of the filters
             * @param gain Gain multiplier for each of the filters
             * @param sampleRate Samplerate the filterbank will be processed at.
             */
            void setParameters(const std::vector<ControllerValue>& centerFrequency, const std::vector<ControllerValue>& bandWidth, const std::vector<ControllerValue>& gain)
			{
				mFilterBank.setParameters(centerFrequency, bandWidth, gain, getSampleRate());
			}

            /**
             * Set the low shelf parallel lowpass filter. This can be used to add the low end that falls outside the bandpass filter bands. Useful if the filterbank is used as EQ.
             * @param cutoffFrequency Cutoff frequency of the low shelf lowpass filter. (normally below the lowest band)
             * @param gain Gain of the low shelf band
             */
            void setLowShelf(ControllerValue cutoffFrequency, ControllerValue gain)
			{
				mFilterBank.setLowShelf(cutoffFrequency, gain, getSampleRate());
			}

        private:
            void process() override;

			FilterBank mFilterBank;
        };

    }
    
}



