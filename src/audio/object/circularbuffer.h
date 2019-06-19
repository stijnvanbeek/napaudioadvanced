#pragma once

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/utility/safeptr.h>
#include <audio/core/audioobject.h>
#include <audio/node/circularbuffernode.h>

namespace nap
{
    
    namespace audio
    {
    
        class CircularBufferInstance;
        
        
        class NAPAPI CircularBuffer : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
            
        public:
            CircularBuffer() : AudioObject() { }
            
        public:
            // Properties
            ResourcePtr<AudioObject> mInput; ///<  property: 'Input' The object whose audio output to rout to the circular buffer.
            
            std::vector<int> mChannelRouting = { 0 }; ///< property: 'ChannelRouting' The size of this vector indicates the number of channels in the circular buffer.
            ///< Each element in the array represents one channel of the circular buffer.
            ///< The value of the element indicates the channel from the input that will be routed to the corresponding channel.

            bool mRootProcess = true;
            
            int mBufferSize = 65536;
            
        private:
            // Inherited from AudioObject
            std::unique_ptr<AudioObjectInstance> createInstance(AudioService& audioService, utility::ErrorState& errorState) override;
        };

        
        class NAPAPI CircularBufferInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
        public:
            CircularBufferInstance() : AudioObjectInstance() { }
            
            // Inherited from AudioObjectInstance
            bool init(AudioObjectInstance& input, const std::vector<int>& channelRouting, bool rootProcess, int bufferSize, AudioService& audioService, utility::ErrorState& errorState);
            OutputPin* getOutputForChannel(int channel) override { return nullptr; }
            int getChannelCount() const override { return 0; }
            
            CircularBufferNode* getChannel(unsigned int channel);
            
        private:
            std::vector<SafeOwner<CircularBufferNode>> mNodes; // Circular buffer for each channel
        };
        
    }
        
}
