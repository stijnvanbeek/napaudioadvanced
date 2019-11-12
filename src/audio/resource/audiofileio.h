#pragma once

// Third party includes
#include <sndfile.h>

// Nap includes
#include <nap/resource.h>
#include <rtti/factory.h>

// Audio includes
#include <audio/core/audionodemanager.h>


namespace nap
{

    namespace audio
    {

        // Class used to wrap the SNDFILE* descriptor in a SafeOwner.
        class NAPAPI AudioFileDescriptor
        {
        public:
            enum class Mode { READ, WRITE, READWRITE };

        public:
            AudioFileDescriptor(const std::string& path, Mode mode, int channelCount = 1, float sampleRate = 44100.f);
            ~AudioFileDescriptor() { sf_close(mSndFile); }
            bool isValid() { return mSndFile != nullptr; }

            /**
             * Writes multichannel interleaved data to the file.
             * @param buffer A vector containing multichannel interleaved audio sample data. The size of the buffer is required to be a multiple of the number of channels in the file.
             */
            unsigned int write(float* buffer, int size);

            /**
             * Reads multichannel interleaved data from the file.
             * @param buffer A vector containing multichannel interleaved audio sample data. The size of the buffer is required to be a multiple of the number of channels in the file.
             */
            unsigned int read(float* buffer, int size);

            int getChannelCount() const { return mSfInfo.channels; }
            float getSampleRate() const { return mSfInfo.samplerate; }
            Mode getMode() const { return mMode; }

        private:
            SNDFILE* mSndFile = nullptr;
            SF_INFO mSfInfo;
            Mode mMode = Mode::WRITE;
        };


        class NAPAPI AudioFileIO : public Resource {
            RTTI_ENABLE(Resource)

        public:
            AudioFileIO(NodeManager& nodeManager) : mNodeManager(nodeManager) { }
            bool init(utility::ErrorState& errorState) override;

            std::string mPath = "";
            AudioFileDescriptor::Mode mMode = AudioFileDescriptor::Mode::WRITE;
            int mChannelCount = 1;

            SafePtr<AudioFileDescriptor> getDescriptor() { return mAudioFileDescriptor; }

        private:
            NodeManager& mNodeManager;
            SafeOwner<AudioFileDescriptor> mAudioFileDescriptor = nullptr;
        };


        using AudioFileIOObjectCreator = rtti::ObjectCreator<AudioFileIO, NodeManager>;


    }

}