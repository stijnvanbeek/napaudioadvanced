/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/core/audioobject.h>


namespace nap
{
    
    namespace audio
    {

        // Forward declarations
        template <typename NodeType> class NodeObjectInstance;


        /**
         * AudioObject whose instance wraps a Node of type NodeType.
         * @tparam NodeType The type of the Node that will be wrapped by this object's instance.
         */
        template <typename NodeType>
        class NodeObject : public AudioObject
        {
            RTTI_ENABLE(AudioObject)

        public:
            NodeObject() : AudioObject() { }
            
        private:
            /**
             * This method can be overridden by descendants to add custom behaviour to the initialization of the wrapped node when this resource is instantiated.
             * @param node The node that is being initialized.
             * @param errorState Logs errors during the initialization
             * @return True on sucess.
             */
            virtual bool initNode(NodeType& node, utility::ErrorState& errorState) { return true; }

            // Inherited from AudioObject
            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;
        };


        /**
         * Non templated base class for NodeObjectInstance
         */
        class NodeObjectInstanceBase : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
        public:
            NodeObjectInstanceBase() = default;
            NodeObjectInstanceBase(const std::string& name) : AudioObjectInstance(name) { }

            /**
             * @return Non-typed pointer to the wrapped node.
             */
            virtual Node* getNonTyped() = 0;
        };
        

        /**
         * Instance of NodeObject. Wraps a single Node of type NodeType.
         * @tparam NodeType type of the wrapped node.
         */
        template <typename NodeType>
        class NodeObjectInstance : public NodeObjectInstanceBase
        {
            RTTI_ENABLE(NodeObjectInstanceBase)
            
        public:
            NodeObjectInstance() = default;
            NodeObjectInstance(const std::string& name) : NodeObjectInstanceBase(name) { }

            /**
             * Initializes this instance object by constructing the wrapped node.
             * @param nodeManager The NodeManager that will process the wrapped node.
             * @errorState Logs errors during the initialization
             * @return True on success/
             */
            virtual bool init(NodeManager& nodeManager, utility::ErrorState& errorState)
            {
                mNode = nodeManager.makeSafe<NodeType>(nodeManager);
                return true;
            }

            // Inherited from AudioObjectInstance
            OutputPin* getOutputForChannel(int channel) override;
            int getChannelCount() const override { return mNode->getOutputs().size();; }
            void connect(unsigned int channel, OutputPin& pin) override;
			void disconnect(unsigned int channel, OutputPin& pin) override;
            int getInputChannelCount() const override { return mNode->getInputs().size(); }

            /**
             * @return SafePtr to the wrapped Node.
             */
            SafePtr<NodeType> get() { return mNode.get(); }

            /**
             * @return Raw pointer to the wrapped Node.
             */
            NodeType* getRaw() { return mNode.getRaw(); }

            /**
             * @return Non types raw pointer to the wrapped Node.
             */
            Node* getNonTyped() override { return mNode.getRaw(); }

        private:
            SafeOwner<NodeType> mNode = nullptr;
        };


        /**
         * Non templated base class for ParallelNodeObject
         */
        class NAPAPI ParallelNodeObjectBase : public AudioObject
        {
            RTTI_ENABLE(AudioObject)

        public:
            ParallelNodeObjectBase() = default;
            ResourcePtr<AudioObject> mInput = nullptr; ///< Property: 'Input' AudioObject that generates the input for the object that owns the InputResourceContainer. Not all ParallelNodeObjects have input, and this property is optional.
            int mChannelCount = 1;                     ///< Property: 'ChannelCount' The number of channels
        };


        /**
         * AudioObject that manages a single input, single output (mono) Node of type NodeType to process each channel.
         * @tparam NodeType The type of the nodes that process one single channel of the NodeObject.
         */
        template <typename NodeType>
        class ParallelNodeObject : public ParallelNodeObjectBase
        {
            RTTI_ENABLE(ParallelNodeObjectBase)

        public:
            ParallelNodeObject() = default;

            // Inherited from AudioObject
            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;

        private:
            /**
             * Override this method to add custom initialization behaviour for each channel's node.
             * @param channel The channel that the node will process.
             * @param node The node being initialized.
             * @param errorState Logs errors during the node's initialization.
             * @return True on sucess.
             */
            virtual bool initNode(int channel, NodeType& node, utility::ErrorState& errorState) { return true; }
        };


        /**
         * Non templated base class for ParallelNodeObjectInstance
         */
        class ParallelNodeObjectInstanceBase : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
        public:
            /**
             * Pure virtual method that has to be overridden to return a non typed raw pointer to the node that processes a given channel.
             * @param channel The channel for which the processing node is requested.
             * @return Non typed raw pointer to the node that processes a given channel.
             */
            virtual Node* getChannelNonTyped(int channel) = 0;
        };


        /**
         * Instance of ParallelNodeObject.
         * AudioObject that manages a single input, single output (mono) Node of type NodeType to process each channel.
         * @tparam NodeType
         */
        template <typename NodeType>
        class ParallelNodeObjectInstance : public ParallelNodeObjectInstanceBase
        {
            RTTI_ENABLE(ParallelNodeObjectInstanceBase)

        public:
            ParallelNodeObjectInstance() = default;

            // Inherited from ParallelNodeObjectInstanceBase
            Node* getChannelNonTyped(int channel) override { return channel < mChannels.size() ? mChannels[channel].getRaw() : nullptr; }

            /**
             * Initializes the ParallelNodeObjectInstance by constructing a node of type NodeType for each channel.
             * @param channelCount Number of processing channels.
             * @param nodeManager The NodeManager this ParallelNodeObjectInstance will be processed on.
             * @param errorState Logs errors during the initialization.
             * @return True on sucsess.
             */
            bool init(int channelCount, NodeManager& nodeManager, utility::ErrorState& errorState);

            /**
             * @return A pointer to the DSP node for the specified channel. Returns nullptr if the channel; is out of bounds.
             */
            NodeType* getChannel(unsigned int channel) { return channel < mChannels.size() ? mChannels[channel].getRaw() : nullptr; }

            /**
             * Clear the processing channels.
             */
            void clear() { mChannels.clear(); }

            // Inherited from AudioObjectInstance
            OutputPin* getOutputForChannel(int channel) override { return *mChannels[channel]->getOutputs().begin(); }
            int getChannelCount() const override { return mChannels.size(); }
            void connect(unsigned int channel, OutputPin& pin) override { (*mChannels[channel]->getInputs().begin())->connect(pin); }
			void disconnect(unsigned int channel, OutputPin& pin) override { (*mChannels[channel]->getInputs().begin())->disconnect(pin); }
            int getInputChannelCount() const override { return (mChannels[0]->getInputs().size() >= 1) ? mChannels.size() : 0; }

        private:
            std::vector<SafeOwner<NodeType>> mChannels;
        };


        // Template definitions

        template <typename NodeType>
        std::unique_ptr<AudioObjectInstance> NodeObject<NodeType>::createInstance(NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            auto instance = std::make_unique<NodeObjectInstance<NodeType>>();
            if (!instance->init(nodeManager, errorState))
                return nullptr;
            if (!initNode(*instance->get(), errorState))
                return nullptr;
            return std::move(instance);
        }


        template <typename NodeType>
        OutputPin* NodeObjectInstance<NodeType>::getOutputForChannel(int channel)
        {
            auto i = 0;
            for (auto& output : mNode->getOutputs())
            {
                if (i == channel)
                    return output;
                i++;
            }
            return nullptr;
        }


        template <typename NodeType>
        void NodeObjectInstance<NodeType>::connect(unsigned int channel, OutputPin& pin)
        {
            auto i = 0;
            for (auto& input : mNode->getInputs())
            {
                if (i == channel)
                {
                    input->connect(pin);
                    return;
                }
                i++;
            }
        }


		template <typename NodeType>
		void NodeObjectInstance<NodeType>::disconnect(unsigned int channel, OutputPin& pin)
		{
			auto i = 0;
			for (auto& input : mNode->getInputs())
			{
				if (i == channel)
				{
					input->disconnect(pin);
					return;
				}
				i++;
			}
		}


		template <typename NodeType>
        std::unique_ptr<AudioObjectInstance> ParallelNodeObject<NodeType>::createInstance(NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            // Initialize the instance
            auto instance = std::make_unique<ParallelNodeObjectInstance<NodeType>>();
            if (!instance->init(mChannelCount, nodeManager, errorState))
                return nullptr;

            // Initialize channel nodes
            for (auto channel = 0; channel < instance->getChannelCount(); ++channel)
                if (!initNode(channel, *instance->getChannel(channel), errorState))
                {
                    errorState.fail("Failed to init node at channel %i", channel);
                    return nullptr;
                }

            // Connect the input
            if (mInput != nullptr)
            {
                if (instance->getInputChannelCount() == 0)
                {
                    errorState.fail("Failed to init ParallelNodeObject: Input property specified but Node has no inputs");
                    return nullptr;
                }
                for (auto channel = 0; channel < instance->getInputChannelCount(); ++channel)
                    instance->connect(channel, *mInput->getInstance()->getOutputForChannel(channel % mInput->getInstance()->getChannelCount()));
            }

            return std::move(instance);
        }


        template <typename NodeType>
        bool ParallelNodeObjectInstance<NodeType>::init(int channelCount, NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            for (auto channel = 0; channel < channelCount; ++channel)
            {
                auto node = nodeManager.makeSafe<NodeType>(nodeManager);

                if (node == nullptr)
                {
                    errorState.fail("Failed to create node.");
                    return false;
                }

                mChannels.emplace_back(std::move(node));
            }

            return true;
        }


    }
    
}

