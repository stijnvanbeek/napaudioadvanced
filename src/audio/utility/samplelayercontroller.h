/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <audio/object/sampler.h>

namespace nap
{

    namespace audio
    {

        /**
         * Utility class to fade looping layers of soundfiles in and out
         */
        class NAPAPI SampleLayerController
        {
        public:
            /**
             * Constructor
             * @param sampler SamplePlayer that will be controlled by this object
             */
            SampleLayerController(SamplePlayerInstance& sampler);

            /**
             * Start a layer
             * @param samplerEntryIndex Index of a sampler entry managed by the SamplePlayer
             * @param attack Attack or fade in time in ms
             */
            void startLayer(int samplerEntryIndex, TimeValue attack);

            /**
             * Stop a playing a layer
             * @param samplerEntryIndex Index of a sampler entry managed by the SamplePlayer
             * @param release Fade out or release time in ms
             */
            void stopLayer(int samplerEntryIndex, TimeValue release);

            /**
             * Stop all playing layers
             * @param release Fade out or release time in ms
             */
            void stopAllLayers(TimeValue release);

            /**
             * Starts a layer for each sampler entry in the set samplerEntryIndices that is not already playing.
             * Stops all currently playing layers that are not in the set samplerEntryIndices
             * @param samplerEntryIndices
             * @param attack Attack or fade in time in ms of layers that will be started
             * @param release Release or fade out time in ms of layers that will be stopped
             */
            void replaceLayers(std::set<int> samplerEntryIndices, TimeValue attack, TimeValue release);

            /**
             * Play a sound and specify when to fade it out again
             * @param samplerEntryIndex Index of a sampler entry from the Sampler object
             * @param attack Attack or fade in time in ms
             * @param sustain Time between fade in and fade out in ms
             * @param release Fade out time in ms
             */
            void play(int samplerEntryIndex, TimeValue attack, TimeValue sustain, TimeValue release);

            /**
             * @return Return sampler entry indices of currently playing layers
             */
            std::set<int> getLayers();

        private:
			SamplePlayerInstance& mSampler;
            EnvelopeNode::Envelope& mEnvelopeData;
            std::map<int, VoiceInstance*> mLayerVoices;
        };

    }

}