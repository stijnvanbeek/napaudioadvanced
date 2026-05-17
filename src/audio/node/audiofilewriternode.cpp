/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "audiofilewriternode.h"

// Audio includes
#include <audio/core/audionodemanager.h>

namespace nap
{

    namespace audio
    {


        AudioFileWriterNode::AudioFileWriterNode(NodeManager& nodeManager, int bufferQueueSize) : Node(nodeManager)
        {
            mBufferQueue.resize(bufferQueueSize);
            for (auto& buffer : mBufferQueue)
                buffer.resize(nodeManager.getInternalBufferSize());
            mBufferSizeInBytes = sizeof(float) * getBufferSize();
            mThread.start();
        }


        AudioFileWriterNode::~AudioFileWriterNode()
        {
        }


		void AudioFileWriterNode::setAudioFile(const SafePtr<AudioFileDescriptor>& audioFileDescriptor)
		{
			assert(audioFileDescriptor != nullptr);
			assert(mActive == 0); // cannot set file descriptor while active
			mAudioFileDescriptor = audioFileDescriptor;
		}


		void AudioFileWriterNode::setActive(bool active)
		{
			assert(mAudioFileDescriptor != nullptr); // cannot activate/deactivate before file descriptor is set

			if (active)
				mActive = 1;
			else
				mActive = 0;
		}


        void AudioFileWriterNode::bufferSizeChanged(int size)
        {
			mThread.stop();
			for (auto& buffer : mBufferQueue)
				buffer.resize(size);
			mBufferSizeInBytes = sizeof(float) * getBufferSize();
			mInputIndex = 0;
			mDiskWriteIndex = 0;
			mThread.start();
        }


        void AudioFileWriterNode::process()
        {
            auto inputBuffer = audioInput.pull();
			auto& outputBuffer = getOutputBuffer(audioOutput);
			outputBuffer = *inputBuffer; // Copy throughput

            std::memcpy(mBufferQueue[mInputIndex].data(), inputBuffer->data(), mBufferSizeInBytes);
            mThread.enqueue([&](){
                if (mActive > 0 && mAudioFileDescriptor != nullptr)
                    mAudioFileDescriptor->write(mBufferQueue[mDiskWriteIndex].data(), getBufferSize());
                mDiskWriteIndex++;
                if (mDiskWriteIndex >= mBufferQueue.size())
                    mDiskWriteIndex = 0;
            });
            mInputIndex++;
            if (mInputIndex >= mBufferQueue.size())
                mInputIndex = 0;
        }

    }

}
