/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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


		void AudioFileReaderNode::setPlaying(bool value)
		{
			assert(mAudioFileDescriptor != nullptr);
			mPlaying = value ? 1 : 0;
		}


        void AudioFileReaderNode::setAudioFile(const SafePtr<AudioFileDescriptor>& audioFileDescriptor)
        {
			assert(audioFileDescriptor != nullptr);
			assert(mPlaying == 0); // cannot set audio file descriptor while playing
            mAudioFileDescriptor = audioFileDescriptor;
            mWritePosition = 0;
            mReadPosition = 0;
			auto framesRead = mAudioFileDescriptor->read(&mCircularBuffer[0], mDiskReadBuffer.size());
			if (framesRead != mDiskReadBuffer.size())
			{
				if (mLooping)
					mAudioFileDescriptor->seek(0);
				else
					mPlaying = 0;
			}
			mWritePosition = framesRead;
        }


        void AudioFileReaderNode::process()
        {
            auto& outputBuffer = getOutputBuffer(audioOutput);

            if (mPlaying == 0)
            {
                for (auto i = 0; i < outputBuffer.size(); ++i)
                    outputBuffer[i] = 0.f;
                return;
            }

            for (auto i = 0; i < outputBuffer.size(); ++i)
            {
                if (mReadPosition < mWritePosition)
                {
                    int mReadPositionFloor = int(mReadPosition);
                    float remainder = mReadPosition - mReadPositionFloor;
                    SampleValue start = mCircularBuffer[wrap(mReadPositionFloor, mCircularBuffer.size())];
                    SampleValue end = mCircularBuffer[wrap(mReadPositionFloor + 1, mCircularBuffer.size())];
                    auto value = lerp(start, end, remainder);
                    outputBuffer[i] = value;
                    mReadPosition += mAudioFileDescriptor->getSampleRate() / getNodeManager().getSampleRate();
                }
                else
                    outputBuffer[i] = 0.f;
            }
            if (mReadPosition > mWritePosition - mDiskReadBuffer.size())
            {
               mThread.enqueue([&](){
                   if (mPlaying == 0)
                       return;
                   auto framesRead = mAudioFileDescriptor->read(mDiskReadBuffer.data(), mDiskReadBuffer.size());
                   if (framesRead != mDiskReadBuffer.size())
                   {
                       if (mLooping)
                           mAudioFileDescriptor->seek(0);
                       else
                           mPlaying = 0;
                   }
                   int wrappedWritePos = wrap(mWritePosition, mCircularBuffer.size());
                   for (auto i = 0; i < framesRead; ++i)
                   {
                       mCircularBuffer[wrappedWritePos] = mDiskReadBuffer[i];
                       wrappedWritePos++;
                       if (wrappedWritePos >= mCircularBuffer.size())
                           wrappedWritePos = 0;
                   }
                   mWritePosition += framesRead;
               });
            }
        }

    }

}
