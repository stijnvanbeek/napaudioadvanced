/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/audioobject.h>
#include <audio/node/audiofilewriternode.h>

namespace nap
{

    namespace audio
    {

        /**
         * AudioObject that writes its audio input to audio file(s)
         */
        class NAPAPI AudioFileWriter : public AudioObject
        {
            RTTI_ENABLE(AudioObject)

        public:
            AudioFileWriter() = default;

            std::vector<ResourcePtr<AudioFileIO>> mAudioFiles; ///< Property: 'AudioFiles' Vector that points to mono @AudioFileWriter resources to write each channel of the object into.
            ResourcePtr<AudioObject> mInput = nullptr;         ///< Property: 'Input' Object where the AudioFileWriter receives its audio input from.

        private:
            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;
        };


        /**
         * Instance of AudioFileWriter
         */
        class NAPAPI AudioFileWriterInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)

        public:
            AudioFileWriterInstance() = default;
            AudioFileWriterInstance(const std::string& name) : AudioObjectInstance(name) { }

            /**
             * Initializes the AudioFileWriterInstance
             * @param nodeManager The NodeManager the processing runs on
             * @param audioFiles An AudioFIleIO audio file descriptor for each channel. Currently only writing mono files per channel is supported.
             * @param input Pointer to AudioObjectInstance providing audio input to record to disk.
             * @param errorState Logs errors during the initialization.
             * @return True on success
             */
            bool init(NodeManager& nodeManager, std::vector<ResourcePtr<AudioFileIO>>& audioFiles, AudioObjectInstance* input, utility::ErrorState& errorState);

            // Inherited from AudioObjectInstance
            int getChannelCount() const override { return 0; }
            OutputPin* getOutputForChannel(int channel) override { return nullptr; }
            int getInputChannelCount() const override { return mNodes.size(); }
            void connect(unsigned int channel, OutputPin& pin) override { mNodes[channel]->audioInput.connect(pin); }
			void disconnect(unsigned int channel, OutputPin& pin) override { mNodes[channel]->audioInput.disconnect(pin); }

            /**
             * Starts or stops recording to disk
             * @param active True to start recording, false to stop it.
             */
            void setActive(bool active);

            /**
             * @return true if the AudioFileWriter is currently recording to disk.
             */
            bool isActive() const { return (*mNodes.begin())->isActive(); }

        private:
            std::vector<ResourcePtr<AudioFileIO>> mAudioFiles;
            std::vector<SafeOwner<AudioFileWriterNode>> mNodes;
        };

    }

}