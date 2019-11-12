#include "audiofilereadernode.h"

// Audio includes
#include <audio/core/audionodemanager.h>

namespace nap
{

    namespace audio
    {


        AudioFileReaderNode::AudioFileReaderNode(NodeManager& nodeManager, unsigned int bufferSize) : Node(nodeManager)
        {
            mCircularBuffer.resize(bufferSize);
            mDiskReadBuffer.resize(getBufferSize());

            mThread.start();
        }


        void AudioFileReaderNode::setAudioFile(SafePtr<AudioFileDescriptor> audioFileDescriptor)
        {
            mAudioFileDescriptor = audioFileDescriptor;
            mWritePosition = 0;
            mReadPosition = 0;
            mAudioFileDescriptor->read(&mCircularBuffer[0], mDiskReadBuffer.size());
            mWritePosition = mDiskReadBuffer.size();
        }


        void AudioFileReaderNode::process()
        {
            auto& outputBuffer = getOutputBuffer(audioOutput);

            for (auto i = 0; i < outputBuffer.size(); ++i)
            {
                int mReadPositionFloor = int(mReadPosition);
                float remainder = mReadPosition - mReadPositionFloor;
                SampleValue start = mCircularBuffer[wrap(mReadPositionFloor, mCircularBuffer.size())];
                SampleValue end = mCircularBuffer[wrap(mReadPositionFloor + 1, mCircularBuffer.size())];
                auto value = lerp(start, end, remainder);
                outputBuffer[i] = value;
                mReadPosition += mAudioFileDescriptor->getSampleRate() / getNodeManager().getSampleRate();
            }
            if (mReadPosition > mWritePosition - mDiskReadBuffer.size())
            {
               mThread.enqueue([&](){
                   mAudioFileDescriptor->read(mDiskReadBuffer.data(), mDiskReadBuffer.size());
                   int wrappedWritePos = wrap(mWritePosition, mCircularBuffer.size());
                   for (auto i = 0; i < mDiskReadBuffer.size(); ++i)
                   {
                       mCircularBuffer[wrappedWritePos] = mDiskReadBuffer[i];
                       wrappedWritePos++;
                       if (wrappedWritePos >= mCircularBuffer.size())
                           wrappedWritePos = 0;
                   }
                   mWritePosition += mDiskReadBuffer.size();
               });
            }
        }

    }

}
