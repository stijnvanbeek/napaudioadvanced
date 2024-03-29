/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Nap includes
#include <utility/threading.h>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/resource/audiofileio.h>
#include <audio/utility/audiofunctions.h>

namespace nap
{

    namespace audio
    {

		/**
		* Node used to read an audio signal from an audio file using an @AudioFileDescriptor.
		*/
		class NAPAPI AudioFileReaderNode : public Node
        {
            RTTI_ENABLE(Node)

        public:
            AudioFileReaderNode(NodeManager& nodeManager, unsigned int bufferSize);

			/**
			 * Sets the audio file descriptor. Needs to ba called before starting playback.
			 * @param audioFileDescriptor to read audio from
			 */
            void setAudioFile(const SafePtr<AudioFileDescriptor>& audioFileDescriptor);

			/**
			 * Starts playback. setAudioFile() needs to be called first.
			 */
			void setPlaying(bool value);

			/**
			 * @return wether the node is currently playing back.
			 */
            bool isPlaying() const { return mPlaying > 0; }

			/**
			 * Specifies if the audio file will loop.
			 * @param value True if the audio file will loop, false if not
			 */
            void setLooping(bool value) { mLooping = value ? 1 : 0; }

			/**
			 * @return whether the audio file is looping.
			 */
            bool isLooping() const { return mLooping > 0; }

            /**
             * Connect this pin to another node's input
             */
            OutputPin audioOutput = { this };

        private:
            void process() override;

            WorkerThread mThread;
            SafePtr<AudioFileDescriptor> mAudioFileDescriptor = nullptr;
            SampleBuffer mCircularBuffer;
            SampleBuffer mDiskReadBuffer;
            DiscreteTimeValue mWritePosition = 0;
            double mReadPosition = 0;
			std::atomic<int> mPlaying = { 0 };
            std::atomic<int> mLooping = { 0 };

        };


    }

}