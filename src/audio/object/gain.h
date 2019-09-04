#pragma once

// Audio includes
#include <audio/core/nodeobject.h>
#include <audio/node/gainnode.h>
#include <nap/resourceptr.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Multichannel audio object to apply a gain to the input channels.
         * Multiple audio inputs will be multiplied with each other and with a scalar.
         */
        class Gain : public MultiChannel<GainNode>
        {
            RTTI_ENABLE(MultiChannel<GainNode>)
            
        public:
            Gain() = default;
            
            int mChannelCount = 1; ///< property: 'ChannelCount' the number of output channels
            std::vector<ControllerValue> mGain = { 1.f }; ///< property: 'Gain' array of gain values per output channel. If the size of the array is less than the number of channels it will be repeated.
            std::vector<ResourcePtr<AudioObject>> mInputs; ///< property: Inputs array of objects used as inputs.
            
        private:
            bool initNode(int channel, GainNode& node, utility::ErrorState& errorState) override;
        };
        
        
    }
    
}

