#pragma once

// Nap includes
#include <utility/threading.h>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/resource/audiofileio.h>

namespace nap
{

    namespace audio
    {


		/**
		 * Node used to write an audio signal to an audio file using an @AudioFileDescriptor.
		 */
        class NAPAPI AudioFileWriterNode : public Node
        {
            RTTI_ENABLE(Node)

        public:
            AudioFileWriterNode(NodeManager& nodeManager, int bufferQueueSize = 4, bool rootProcess = true);
            ~AudioFileWriterNode();

			/**
			 * Sets the audio file descriptor to write to. Warning: ony call this when the node is not active.
			 * @param audioFileDescriptor the descriptor to write to.
			 */
            void setAudioFile(const SafePtr<AudioFileDescriptor>& audioFileDescriptor);

			/**
			 * Activates/deactivates writing to disk
			 */
			void setActive(bool active);

			/**
			 * Indicates wether the node is writing to disk
			 */
			 bool isActive() const { return mActive > 0; }

            InputPin audioInput = { this };

        private:
            void process() override;
            void bufferSizeChanged(int) override;

            std::vector<SampleBuffer> mBufferQueue;
            int mInputIndex = 0;
            int mDiskWriteIndex = 0;
            WorkerThread mThread;
            int mBufferSizeInBytes = 0;
            SafePtr<AudioFileDescriptor> mAudioFileDescriptor = nullptr;
            bool mRootProcess = false;

			std::atomic<int> mActive = { 0 }; // Indicates wether the node is active. Active when greater than zero.
        };


    }

}