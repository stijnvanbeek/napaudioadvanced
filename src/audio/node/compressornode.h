/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Audio includes
#include <audio/core/audionode.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Compressor algorithm class compiled Faust code.
         *
         * Ported faust code:
         *
         * import("stdfaust.lib");
         * ratio = vslider("ratio", 4, 1, 20, 0.01);
         * thresh = vslider("thresh", -6, -90, 0, 0.1);
         * attack = vslider("attack", 0.0008, 0, 0.2, 0.0001);
         * release = vslider("release", 0.5, 0, 1, 0.0001);
         *
         * process = co.compressor_mono(ratio, thresh, attack, release);
         */
        class NAPAPI FaustCompressor
        {
        public:
            /**
             * Constructor
             * @param samplerate Processing sample rate
             */
            FaustCompressor(int samplerate);

            /**
             * Sets the attack time
             * @param attack Attack time in ms, between 0.0 and 0.2ms.
             */
            void setAttack(float attack) { fVslider0 = attack; }

            /**
             * Sets the ratio
             * @param ratio Ratio between 1 and 20.
             */
            void setRatio(float ratio) { fVslider1 = ratio; }

            /**
             * Sets the release in ms
             * @param release In ms between 0. and 1.
             */
            void setRelease(float release) { fVslider2 = release; }

            /**
             * Sets the threshold in dB
             * @param threshold in dB between -90 and 0dB.
             */
            void setThreshold(float threshold) { fVslider3 = threshold; }

            /**
             * Processes a number of samples
             * @param count Number of samples to be processed.
             * @param input0 Pointer to buffer with count number of input samples.
             * @param output0 Pointer to buffer with count number of samples that the output will be written to.
             */
            void compute(int count, float* input0, float* output0);
            
        private:
            int fSamplingFreq;
            
            std::atomic<float> fVslider0 = 0.001; // attack
            std::atomic<float> fVslider1 = 4; // ratio
            std::atomic<float> fVslider2 = 0.5; // release
            std::atomic<float> fVslider3 = -6; // threshold
            
            float fConst0 = 0.f;
            float fConst1 = 0.f;
            float fConst2 = 0.f;
            
            float fRec2[2] = { 0.f, 0.f };
            float fRec1[2] = { 0.f, 0.f };
            float fRec0[2] = { 0.f, 0.f };
        };
        
        /**
         * CompressorNode wraps the FaustCompressor.
         */
        class NAPAPI CompressorNode : public Node
        {
            RTTI_ENABLE(Node)
        public:
            CompressorNode(NodeManager& manager) : Node(manager), faustCompressor(getSampleRate()) {
                setRatio(4.);
                setThreshold(-6.);
                setAttack(0.0008);
                setRelease(0.5);
            }
            
            InputPin audioInput = { this };     ///< Audio input pin
            OutputPin audioOutput = { this };   ///< Audio output pin

            /**
             * Sets the attack time
             * @param attack Attack time in ms, between 0.0 and 0.2ms.
             */
            void setAttack(float attack) { faustCompressor.setAttack(attack); }

            /**
             * Sets the ratio
             * @param ratio Ratio between 1 and 20.
             */
            void setRatio(float ratio) { faustCompressor.setRatio(ratio); }

            /**
             * Sets the release in ms
             * @param release In ms between 0. and 1.
             */
            void setRelease(float release) { faustCompressor.setRelease(release); }

            /**
             * Sets the threshold in dB
             * @param threshold in dB between -90 and 0dB.
             */
            void setThreshold(float threshold) { faustCompressor.setThreshold(threshold); }
            
        private:
            void process() override;
            
            FaustCompressor faustCompressor;
            
        };
        
    }
}

