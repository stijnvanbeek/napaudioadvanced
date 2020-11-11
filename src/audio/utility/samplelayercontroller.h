#pragma once

#include <audio/object/sampler.h>

namespace nap
{

    namespace audio
    {

        class NAPAPI SampleLayerController
        {
        public:
            SampleLayerController(SamplePlayerInstance& sampler);

            void startLayer(int samplerEntryIndex, TimeValue attack);
            void stopLayer(int samplerEntryIndex, TimeValue release);
            void stopAllLayers(TimeValue release);
            void replaceLayers(std::set<int> samplerEntrieIndices, TimeValue attack, TimeValue release);
            void play(int samplerEntryIndex, TimeValue attack, TimeValue sustain, TimeValue release);
            std::set<int> getLayers();

        private:
			SamplePlayerInstance& mSampler;
            EnvelopeNode::Envelope& mEnvelopeData;
            std::map<int, VoiceInstance*> mLayerVoices;
        };

    }

}