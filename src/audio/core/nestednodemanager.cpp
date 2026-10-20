/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "nestednodemanager.h"

namespace nap
{

    namespace audio
    {

        void NestedNodeManagerNode::init(int inputChannelCount, int outputChannelCount, int internalBufferSize, int reserveProcesses, int reserveRootProcesses)
        {
            mNestedNodeManager = std::make_unique<NodeManager>(getNodeManager().getDeletionQueue(), reserveProcesses, reserveRootProcesses);

            mNestedNodeManager->setInputChannelCount(inputChannelCount);
            mNestedNodeManager->setOutputChannelCount(outputChannelCount);
            mNestedNodeManager->setSampleRate(getNodeManager().getSampleRate());
            mNestedNodeManager->setInternalBufferSize(internalBufferSize);

            for (auto i = 0; i < outputChannelCount; ++i)
            {
                _mOutputs.emplace_back(std::make_unique<OutputPin>(this));
                mOutputBuffers.emplace_back(nullptr);
            }
            for (auto i = 0; i < inputChannelCount; ++i)
            {
                _mInputs.emplace_back(std::make_unique<InputPin>(this));
                mInputBuffers.emplace_back(nullptr);
            }
        }


        void NestedNodeManagerNode::process()
        {
            for (auto i = 0; i < _mInputs.size(); ++i)
            {
                auto inputBuffer = _mInputs[i]->pull();
                if (inputBuffer == nullptr)
                    mInputBuffers[i] = nullptr;
                else
                    mInputBuffers[i] = inputBuffer;
            }

            for (auto i = 0; i < _mOutputs.size(); ++i)
            {
                auto outputBuffer = &getOutputBuffer(*_mOutputs[i]);
                mOutputBuffers[i] = outputBuffer;
            }
            mNestedNodeManager->process(mInputBuffers, mOutputBuffers, getBufferSize());
        }


        void NestedNodeManagerNode::sampleRateChanged(float sampleRate)
        {
            if (mNestedNodeManager != nullptr)
                mNestedNodeManager->setSampleRate(sampleRate);
        }


        bool NestedNodeManagerInstance::init(NodeManager &nodeManager, int inputChannelCount, int outputChannelCount, int internalBufferSize, int reserveProcesses, int reserveRootProcesses, utility::ErrorState &errorState)
        {
            mNode = nodeManager.makeSafe<NestedNodeManagerNode>(nodeManager);
            mNode->init(inputChannelCount, outputChannelCount, internalBufferSize, reserveProcesses, reserveRootProcesses);
            return true;
        }

    }

}
