/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
            AudioFileWriterNode(NodeManager& nodeManager, int bufferQueueSize = 4);
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
			 * Indicates whether the node is writing to disk
			 */
			 bool isActive() const { return mActive > 0; }

			 /**
			  * Connect another node's audio output to this pin to write it to a file.
			  */
            InputPin audioInput = { this };

			/**
			 * This pin just outputs the inpus.
			 * Connecting to this pin can be an alternative to process this node without registering it as root process.
			 */
			OutputPin audioOutput = { this };

        private:
            void process() override;
            void bufferSizeChanged(int) override;

            std::vector<SampleBuffer> mBufferQueue;
            int mInputIndex = 0;
            int mDiskWriteIndex = 0;
            WorkerThread mThread;
            int mBufferSizeInBytes = 0;
            SafePtr<AudioFileDescriptor> mAudioFileDescriptor = nullptr;

			std::atomic<int> mActive = { 0 }; // Indicates whether the node is active. Active when greater than zero.
        };


    }

}