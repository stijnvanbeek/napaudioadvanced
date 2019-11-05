#include "audiofilewriterobject.h"

RTTI_BEGIN_CLASS(nap::audio::AudioFileWriterObject)
    RTTI_PROPERTY("AudioFileWriters", &nap::audio::AudioFileWriterObject::mAudioFileWriters, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Input", &nap::audio::AudioFileWriterObject::mInput, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AudioFileWriterObjectInstance)
RTTI_END_CLASS


namespace nap
{

    namespace audio
    {

        std::unique_ptr<AudioObjectInstance> AudioFileWriterObject::createInstance(NodeManager &nodeManager, utility::ErrorState &errorState)
        {
            auto instance = std::make_unique<AudioFileWriterObjectInstance>();
            if (!instance->init(nodeManager, mAudioFileWriters, mInput->getInstance(), errorState))
            {
                errorState.fail("Failed to initialize AudioFileWriterObjectInstance");
                return nullptr;
            }

            return instance;
        }


        bool AudioFileWriterObjectInstance::init(NodeManager &nodeManager, std::vector<ResourcePtr<AudioFileWriter>>& audioFileWriters, AudioObjectInstance* input, utility::ErrorState &errorState)
        {
            if (input != nullptr)
                if (input->getChannelCount() < 1)
                {
                    errorState.fail("AudioFileWriterObjectInstance input needs to have at least 1 output channel");
                    return false;
                }


            mAudioFileWriters = audioFileWriters;
            int inputChannel = 0;
            for (auto& audioFileWriter : mAudioFileWriters)
            {
                if (audioFileWriter->mSampleRate != nodeManager.getSampleRate())
                {
                    errorState.fail("Audio file samplerate does not equal system sample rate");
                    return false;
                }
                if (audioFileWriter->mChannelCount != 1)
                {
                    errorState.fail("AudioFileWriterObject works with mono AudioFileWriter resources");
                    return false;
                }

                auto node = nodeManager.makeSafe<AudioFileWriterNode>(nodeManager, 4, true);
                node->setAudioFile(audioFileWriter->getDescriptor());
                if (input != nullptr)
                    node->audioInput.connect(*input->getOutputForChannel(inputChannel % input->getChannelCount()));
                inputChannel++;
                mNodes.emplace_back(std::move(node));
            }

            return true;
        }
    }

}

