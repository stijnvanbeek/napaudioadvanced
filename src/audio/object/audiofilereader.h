/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/audioobject.h>
#include <audio/node/audiofilereadernode.h>

namespace nap
{

    namespace audio
    {

        /**
         * Audio object for reading audio data directly from file(s) on disk
         */
        class NAPAPI AudioFileReader : public AudioObject
        {
            RTTI_ENABLE(AudioObject)

        public:
            AudioFileReader() = default;

            std::vector<ResourcePtr<AudioFileIO>> mAudioFiles; ///< property: 'AudioFiles' Vector that points to mono @AudioFileIO resources to read each channel of the object from.
            int mBufferSize = 65536;                           ///< Property: 'BufferSize' Size of the internal circular buffers of the audio file readers

        private:
            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;
        };


        /**
         * Instance of AudioFileReader
         */
        class NAPAPI AudioFileReaderInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)

        public:
            AudioFileReaderInstance() = default;
            AudioFileReaderInstance(const std::string& name) : AudioObjectInstance(name) { }

            /**
             * Initialize the instance
             * @param nodeManager The NodeManager that the AudioFileReaderNode run on
             * @param audioFiles Audio file descriptors for the audio files that will be read
             * @param bufferSize Buffer size of the audio file reader nodes' internal circular buffers.
             * @param errorState Logs errors during the initialization process
             * @return True on success
             */
            bool init(NodeManager& nodeManager, std::vector<ResourcePtr<AudioFileIO>>& audioFiles, int bufferSize, utility::ErrorState& errorState);

            /**
             * @return The number of audio channels of this object
             */
            int getChannelCount() const override { return mNodes.size(); }

            /**
             * @param channel Index of the requested output channel
             * @return Audio pin for the requested output channel.
             */
            OutputPin* getOutputForChannel(int channel) override { return &mNodes[channel]->audioOutput; }

            /**
             * Starts or stops reading from disk
             * @param playing True is set to playing, false when stopped.
             */
            void setPlaying(bool playing);

            /**
             * @return Wether the object is currently reading and outputting audio from disk
             */
            bool isPlaying() const { return (*mNodes.begin())->isPlaying(); }

            /**
             * @param looping Sets whether the audiofiles that are played back will loop (start all over) after reaching the end of the file.
             */
            void setLooping(bool looping);

            /**
             * @return Whether the audiofiles that are played back will loop (start all over) after reaching the end of the file.
             */
            bool isLooping() const { return (*mNodes.begin())->isLooping(); }

        private:
            std::vector<ResourcePtr<AudioFileIO>> mAudioFiles;
            std::vector<SafeOwner<AudioFileReaderNode>> mNodes;
        };

    }

}