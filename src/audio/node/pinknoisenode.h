#pragma once

#include <audio/core/audionode.h>
#include <audio/utility/onepole.h>

namespace nap
{

    namespace audio
    {

        /**
         * Pink noise generator
         */
        class NAPAPI PinkNoiseNode : public Node
        {
            RTTI_ENABLE(Node)
            
        public:
            PinkNoiseNode(NodeManager& nodeManager) : Node(nodeManager) { }

            OutputPin output = { this };
            
        private:
            void process() override;
            void sampleRateChanged(float sampleRate) override;
            std::vector<std::unique_ptr<OnePoleLowPass<float>>> mFilters;
            std::vector<float> mAmplitudes;
        };
    
    }
}