/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Nap includes
#include <nap/resource.h>
#include <rtti/factory.h>

// Audio includes
#include <audio/core/audionodemanager.h>

// Third party includes
#include <sndfile.h>

namespace nap
{

    class Core;

    namespace audio
    {

        /**
         * Class used to wrap the libsndfile SNDFILE* descriptor in a SafeOwner.
         */
        class NAPAPI AudioFileDescriptor
        {
			RTTI_ENABLE()
        public:
            enum class Mode { READ, WRITE, READWRITE };

        public:
            /**
             * Constructor
             * @param path Path to the audio file
             * @param mode Indicating if the file is opened for reading, created for writing of opened for reading and writing
             * @param channelCount Number of channels if the file is created for writing (Mode::WRITE)
             * @param sampleRate Samplerate if the file is created for writing (Mode::WRITE)
             */
            AudioFileDescriptor(const std::string& path, Mode mode, int channelCount = 1, float sampleRate = 44100.f);
            ~AudioFileDescriptor();

            /**
             * @return If the soundfile is opened or created successfully
             */
            bool isValid() { return mSndFile != nullptr; }

            /**
             * Writes multichannel interleaved data to the file.
             * @param buffer A vector containing multichannel interleaved audio sample data.
             * @param size The size of the buffer is required to be a multiple of the number of channels in the file.
             * @return The number of samples written
             */
            unsigned int write(float* buffer, int size);

            /**
             * Reads multichannel interleaved data from the file.
             * @param buffer A vector containing multichannel interleaved audio sample data.
             * @param size The size of the buffer is required to be a multiple of the number of channels in the file.
             * @return The number of samples read
             */
            unsigned int read(float* buffer, int size);

            /**
             * Moves the read/write position to the given offset.
             */
            void seek(DiscreteTimeValue offset);

            /**
             * @return The number of channels in the audio file
             */
            int getChannelCount() const { return mChannelCount; }

            /**
             * @return The samplerate of the audio file
             */
            float getSampleRate() const { return mSampleRate; }

            /**
             * @return If the file is opened for reading, writing or both
             */
            Mode getMode() const { return mMode; }

        private:
            SNDFILE* mSndFile;
            int mChannelCount = 1;
            float mSampleRate = 44100.f;
            Mode mMode = Mode::WRITE;
        };


        /**
         * Resource wrapping an AudioFileDescriptor
         */
        class NAPAPI AudioFileIO : public Resource {
            RTTI_ENABLE(Resource)

        public:
            AudioFileIO(Core& core);

            // Inherited from Resource
            bool init(utility::ErrorState& errorState) override;

            std::string mPath = "";                                             ///< Property: 'Path' Path to the audio file
            AudioFileDescriptor::Mode mMode = AudioFileDescriptor::Mode::WRITE; ///< Property: 'Mode' Indicates if the file is opened for reading, writing or both
            int mChannelCount = 1;                                              ///< Property: 'ChannelCount' Number of channels if the files is created for writing.

            /**
             * @return Pointer to the audio file descriptor to perform reading or writing
             */
            SafePtr<AudioFileDescriptor> getDescriptor() { return mAudioFileDescriptor; }

        private:
            NodeManager* mNodeManager = nullptr;
            SafeOwner<AudioFileDescriptor> mAudioFileDescriptor = nullptr;
        };


    }

}