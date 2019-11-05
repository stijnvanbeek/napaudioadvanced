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
            AudioFileDescriptor(const char* path, int mode, SF_INFO *sfinfo) { mSndFile = sf_open(path, mode, sfinfo); }
            ~AudioFileDescriptor() { sf_close(mSndFile); }
            bool isValid() { return mSndFile != nullptr; }

            /**
             * Writes multichannel interleaved data to the file.
             * @param buffer A vector containing multichannel interleaved audio sample data. The size of the buffer is required to be a multiple of the number of channels in the file.
             */
            unsigned int write(const std::vector<SampleValue>& buffer);


            SNDFILE* mSndFile = nullptr;
        };


        class NAPAPI AudioFileWriter : public Resource {
            RTTI_ENABLE(Resource)

        public:
            AudioFileWriter(NodeManager& nodeManager) : mNodeManager(nodeManager) { }
            bool init(utility::ErrorState& errorState) override;

            std::string mPath = "";
            float mSampleRate = 44100.f;
            int mChannelCount = 1;

            SafePtr<AudioFileDescriptor> getDescriptor() { return mAudioFileDescriptor; }

        private:
            NodeManager& mNodeManager;
            SafeOwner<AudioFileDescriptor> mAudioFileDescriptor = nullptr;
        };


        using AudioFileWriterObjectCreator = rtti::ObjectCreator<AudioFileWriter, NodeManager>;


    }

}