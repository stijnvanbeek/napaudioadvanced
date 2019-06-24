#pragma once

// Audio includes
#include <audio/core/multichannelobject.h>
#include <audio/node/filternode.h>
#include <nap/resourceptr.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Multichannel audio object to apply a filter to the input channels.
         */
        class Filter : public MultiChannelObject
        {
            RTTI_ENABLE(MultiChannelObject)
            
        public:
            Filter() = default;
            
            FilterNode::EMode mMode = FilterNode::EMode::LowPass;
            std::vector<ControllerValue> mFrequency = { 440.f };
            std::vector<ControllerValue> mResonance = { 0.f };
            std::vector<ControllerValue> mBand = { 100.f };
            std::vector<ControllerValue> mGain = { 1.f };
            int mChannelCount = 1; ///< property: 'ChannelCount' the number of delays and output channels
            ResourcePtr<AudioObject> mInput; ///< property: "Input" @AudioObject whose output channels will be used as inputs for the delay channels.
            
        private:
            SafeOwner<Node> createNode(int channel, AudioService& service, utility::ErrorState& errorState) override;
            
            int getChannelCount() const override { return mChannelCount; }
        };
        
        
    }
    
}

