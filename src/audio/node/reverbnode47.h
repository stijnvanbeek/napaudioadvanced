/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

/* The reverb47 algorithm is named after the Contactweg 47 in Amsterdam,
 * where it was designed and prototyped by Poul Holleman and implemented by Stijn van Beek.*/

#pragma once

// Spatial Audio includes
#include <audio/utility/allpass.h>
#include <audio/utility/comb.h>
#include <audio/utility/onepole.h>
#include <audio/utility/singledelay.h>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/utility/linearsmoothedvalue.h>
#include <audio/utility/dirtyflag.h>

// Nap includes
#include <mathutils.h>

namespace nap
{

    namespace audio
    {

        namespace verb47
        {

            /**
             * All non-parametric "magic numbers" that define the response of the reverb together in a struct
             */
            struct NAPAPI ReverbSettings
            {
                std::vector<float> mInputAllPassDelays = { 4.76f, 3.58f, 12.73f, 9.3f  };
                std::vector<float> mInputAllPassGains = { 0.75f, 0.75f, 0.625f, 0.625f  };
                std::vector<float> mSizeAllPassGains = { -0.7f, 0.5f };
                std::vector<float> mSizeAllPassDelays = { 22.58f, 60.48f };
                std::vector<float> mDelaySizeMultipliers = { 149.6f, 125.f };
                std::vector<float> mDiffusorDelayMultipliers = { 110.f, 160.f, 50.f, 25.f, 69.f, 80.f, 75.f };
                float mGain = 0.30f;

                /**
                 * Multiply all magic numbers indicating a time value
                 * @param factor Multiplication factor
                 */
                void multiply(float factor);
            };


            /**
             * A reverberation algorithm controlled by 4 main parameters: size, decay, damping and diffusion
             * Additionally the pitch of the reverb tail can be modulated with low frequent noise using 3 additional parameters: bandwidth, speed and cutoff frequency.
             *
             * Node that performs a reverberation algorithm that consists of the following processing chain:
             * - Onepole damping lowpass filter, the cutoff frequency is determined by the damping parameter, referred to as InputOnePole
             * - A series of 4 allpass filters tuned to magic numbers, referred to as InputAllPasses
             * - Another allpass filter tuned to the size parameter, referred to as SizeAllPasses[0]
             * - A delay line without feedback referred to as mDelays[0], tuned to the size parameter and modulated by a low frequent noise modulator signal, referred to as mModulator
             * - Another damping onepole lowpass filter, determined by the damping parameter, referred to as DampingOnePole
             * - Attenuation of the signal with the decay parameter
             * - Another allpass filter tuned to the size parameter, SizeAllPasses[1]
             * - Another delay line tuned to the size parameter, Delays[1]. The output of this delay is fed back into SizeAllPasses[0]
             * - A diffusion network consisting of 7 parallel delays, tuned to magic numbers receiving input from different places within the above chain. 3 of the diffusion inputs can be connected to the diffusion outputs of another reverb node, in order to achieve cross diffusion.
             */
            class NAPAPI ReverbNode : public Node
            {
            RTTI_ENABLE(Node)
            public:
                ReverbNode(NodeManager& nodeManager);

                InputPin audioInput = { this };
                OutputPin audioOutput = { this };

                InputPin diffusionInput1 = { this }; ///< Connect another ReverbNode's diffusion output to this for cross diffusion
                InputPin diffusionInput2 = { this }; ///< Connect another ReverbNode's diffusion output to this for cross diffusion
                InputPin diffusionInput3 = { this }; ///< Connect another ReverbNode's diffusion output to this for cross diffusion

                OutputPin diffusionOutput1 = { this }; ///< Connect another ReverbNode's diffusion input to this for cross diffusion
                OutputPin diffusionOutput2 = { this }; ///< Connect another ReverbNode's diffusion input to this for cross diffusion
                OutputPin diffusionOutput3 = { this }; ///< Connect another ReverbNode's diffusion input to this for cross diffusion

                /**
                 * Adjust the room size parameter
                 * @param value normalized between 0 and 1.0
                 */
                void setSize(ControllerValue value);

                /**
                 * Adjust the decay time parameter
                 * @param value normalized between 0 and 1.0
                 */
                void setDecay(ControllerValue value);

                /**
                 * Adjust the damping frequency parameter
                 * @param value normalized between 0 and 1.0
                 */
                void setDamping(ControllerValue value);

                /**
                 * Adjust the damping diffusion parameter
                 * @param value normalized between 0 and 1.0
                 */
                void setDiffusion(ControllerValue value);

                /**
                 * Adjust the modulation bandwidth, the amplitude of the modulation
                 * @param value normalized between 0 and 1.0
                 */
                void setModulationAmplitude(ControllerValue value);

                /**
                 * Adjust the speeds of the modulation
                 * @param value normalized between 0 and 1.0
                 */
                void setModulationSpeed(ControllerValue value);

                /**
                 * Adjust cutting of low frequencies from the input signal
                 * @param value normalized between o and 1
                 */
                void setLowCut(ControllerValue value);

                /**
                 * Applies a new set of magic numbers.
                 * @param settings
                 */
                void applySettings(const ReverbSettings& settings)
                {
                    mSettings = settings;
                    applySettingsToDSP();
                }

            private:
                void process() override;
                void sampleRateChanged(float sampleRate) override;
                void applySettingsToDSP();

                ControllerValue mSize = 0.f;
                ControllerValue mDiffusion = 0.f;
                ControllerValue mDecay = 0.f;
                ControllerValue mDamping = 0.f;
                ControllerValue mInputLowCut = 20.f;
                ControllerValue mModulationBandWidth = 2.f;
                ControllerValue mModulationTime = 200.f;

                OnePoleLowPass<SampleValue> mInputHighCutOnePole;
                OnePoleHighPass<SampleValue> mInputLowCutOnePole;
                OnePoleLowPass<SampleValue> mDampingOnePole;
                std::array<AllPass, 4> mInputAllPasses;
                std::array<AllPass, 2> mSizeAllPasses;
                std::array<SingleDelay, 2> mDelays;
                std::array<SingleDelay, 7> mDiffusors;

                LinearSmoothedValue<ControllerValue> mModulator = { 0.f, 0 };
                OnePoleLowPass<SampleValue> mModulationOnePole;

                float mSamplesPerMillisecond = 1;
                SampleValue mFeedbackInput = 0.f;
                ReverbSettings mSettings;
                DirtyFlag mSettingsDirty;
            };

        }

    }

}