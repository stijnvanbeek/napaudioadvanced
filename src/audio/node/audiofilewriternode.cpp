#include "audiofilewriternode.h"

// Audio includes
#include <audio/core/audionodemanager.h>

namespace nap
{

    namespace audio
    {


        AudioFileWriterNode::AudioFileWriterNode(NodeManager& nodeManager, int bufferQueueSize, bool rootProcess) : Node(nodeManager), mRootProcess(rootProcess)
        {
            mBufferQueue.resize(bufferQueueSize);
            for (auto& buffer : mBufferQueue)
                buffer.resize(nodeManager.getInternalBufferSize());
            mBufferSizeInBytes = sizeof(float) * getBufferSize();
            mThread.start();
            if (mRootProcess)
                nodeManager.registerRootProcess(*this);
        }


        AudioFileWriterNode::~AudioFileWriterNode()
        {
            if (mRootProcess)
                getNodeManager().unregisterRootProcess(*this);
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
            getNodeManager().enqueueTask([&](){
                mThread.stop();
                for (auto& buffer : mBufferQueue)
                    buffer.resize(size);
                mBufferSizeInBytes = sizeof(float) * getBufferSize();
                mInputIndex = 0;
                mDiskWriteIndex = 0;
                mThread.start();
            });
        }


        void AudioFileWriterNode::process()
        {
            auto inputBuffer = audioInput.pull();
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
