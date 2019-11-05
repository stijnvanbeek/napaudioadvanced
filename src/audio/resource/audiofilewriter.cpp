#include "audiofilewriter.h"

// Audio includes
#include <audio/core/audionodemanager.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AudioFileWriter)
    RTTI_PROPERTY("Path", &nap::audio::AudioFileWriter::mPath, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("SampleRate", &nap::audio::AudioFileWriter::mSampleRate, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("ChannelCount", &nap::audio::AudioFileWriter::mChannelCount, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{

    namespace audio
    {

        unsigned int AudioFileDescriptor::write(const std::vector<SampleValue>& buffer)
        {
            return sf_write_float(mSndFile, buffer.data(), buffer.size());
        }


        bool AudioFileWriter::init(utility::ErrorState& errorState)
        {
            SF_INFO sfInfo;
            sfInfo.channels = mChannelCount;
            sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
            sfInfo.samplerate = mSampleRate;
            mAudioFileDescriptor = mNodeManager.makeSafe<AudioFileDescriptor>(mPath.c_str(), SFM_WRITE, &sfInfo);

            if (!mAudioFileDescriptor->isValid())
            {
                errorState.fail("Failed to open audio file %s for writing", mPath.c_str());
                return false;
            }

            return true;
        }


    }
}