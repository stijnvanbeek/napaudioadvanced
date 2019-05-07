#pragma once

#include <audio/core/audionode.h>
#include <audio/utility/audiofunctions.h>
#include <audio/utility/safeptr.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         *
         */
        class NAPAPI CircularBufferNode : public Node
        {
            RTTI_ENABLE(Node)
        public:
            CircularBufferNode(NodeManager& nodeManager, unsigned int bufferSize, bool rootProcess = true);
            virtual ~CircularBufferNode();
            
            InputPin audioInput = { this };
            
            inline const SampleValue& getSample(const DiscreteTimeValue& absolutePosition) const { return mBuffer[wrap(absolutePosition, mBuffer.size())]; }
            DiscreteTimeValue getAbsolutePosition(unsigned int relativePosition) const { return wrap(mWritePosition - relativePosition, mBuffer.size()); }
            
        private:
            void process() override;
             
            SampleBuffer mBuffer;
            DiscreteTimeValue mWritePosition = 0;
            
            bool mRootProcess = false;
        };
        
    }
}
