/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "audiofileio.h"

// Audio includes
#include <audio/core/audionodemanager.h>
#include <audio/service/audioservice.h>

// Nap includes
#include <nap/core.h>

RTTI_BEGIN_ENUM(nap::audio::AudioFileDescriptor::Mode)
    RTTI_ENUM_VALUE(nap::audio::AudioFileDescriptor::Mode::WRITE, "Write"),
    RTTI_ENUM_VALUE(nap::audio::AudioFileDescriptor::Mode::READ, "Read"),
    RTTI_ENUM_VALUE(nap::audio::AudioFileDescriptor::Mode::READWRITE, "ReadWrite")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AudioFileIO)
    RTTI_CONSTRUCTOR(nap::Core&)
    RTTI_PROPERTY("Path", &nap::audio::AudioFileIO::mPath, nap::rtti::EPropertyMetaData::FileLink)
    RTTI_PROPERTY("Mode", &nap::audio::AudioFileIO::mMode, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("ChannelCount", &nap::audio::AudioFileIO::mChannelCount, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{

    namespace audio
    {

        AudioFileDescriptor::AudioFileDescriptor(const std::string& path, Mode mode, int channelCount, float sampleRate)
        {
            mMode = mode;
            SF_INFO sfInfo;
            sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
            if (mode == Mode::WRITE)
            {
                sfInfo.channels = channelCount;
                sfInfo.samplerate = sampleRate;
            }
            int libSndFileMode;
            if (mode == Mode::WRITE)
                libSndFileMode = SFM_WRITE;
            else if (mode == Mode::READ)
                libSndFileMode = SFM_READ;
            else
                libSndFileMode = SFM_RDWR;
            mSndFile = sf_open(path.c_str(), libSndFileMode, &sfInfo);
            mSampleRate = sfInfo.samplerate;
            mChannelCount = sfInfo.channels;
        }


        AudioFileDescriptor::~AudioFileDescriptor()
        {
            sf_close(mSndFile);
        }


        unsigned int AudioFileDescriptor::write(float* buffer, int size)
        {
            return sf_write_float(mSndFile, buffer, size);
        }


        unsigned int AudioFileDescriptor::read(float* destination, int size)
        {
            return sf_read_float(mSndFile, destination, size);
        }


        void AudioFileDescriptor::seek(DiscreteTimeValue offset)
        {
            sf_seek(mSndFile, offset, SEEK_SET);
        }



        AudioFileIO::AudioFileIO(Core &core) : Resource()
        {
            auto audioService = core.getService<AudioService>();
            assert(audioService != nullptr);
            mNodeManager = &audioService->getNodeManager();
        }


        bool AudioFileIO::init(utility::ErrorState& errorState)
        {
            mAudioFileDescriptor = mNodeManager->makeSafe<AudioFileDescriptor>(mPath, mMode, mChannelCount, mNodeManager->getSampleRate());

            if (!mAudioFileDescriptor->isValid())
            {
                errorState.fail("Failed to open audio file %s for writing", mPath.c_str());
                return false;
            }

            return true;
        }


    }
}