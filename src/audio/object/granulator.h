#pragma once

#include <audio/object/circularbuffer.h>
#include <audio/core/audioobject.h>
#include <audio/utility/translator.h>
#include <audio/core/nodeobject.h>
#include <audio/node/outputnode.h>
#include <audio/core/nestednodemanager.h>
#include <audio/core/polyphonic.h>
#include <audio/utility/delay.h>
#include <audio/core/audionode.h>
#include <audio/resource/audiobufferresource.h>

namespace nap
{

    namespace audio
    {


        class StereoGrainPannerNode : public Node
        {
        public:
            StereoGrainPannerNode(NodeManager& nodeManager) : Node(nodeManager), mDelay(32768) { }

            InputPin audioInput = { this };
            InputPin envelopeInput = { this };
            OutputPin leftOutput = { this };
            OutputPin rightOutput = { this };

            void setPanning(ControllerValue panning, ControllerValue amplitude, TimeValue maxDelay);

        private:
            void process() override;

            ControllerValue leftGain = 0;
            ControllerValue rightGain = 0;
            int leftDelayTime = 0;
            int rightDelayTime = 0;
            Delay mDelay;
        };



        class StereoGrainPanner : public AudioObject
        {
        public:
            StereoGrainPanner() = default;

            ResourcePtr<AudioObject> mInput  = nullptr;
            ResourcePtr<Envelope> mEnvelope = nullptr;

        private:
            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;
        };



        class StereoGrainPannerInstance : public AudioObjectInstance
        {
        public:
            StereoGrainPannerInstance() = default;
            StereoGrainPannerInstance(const std::string& name) : AudioObjectInstance(name) { }

            bool init(StereoGrainPanner& resource, NodeManager& nodeManager, utility::ErrorState& errorState);

            OutputPin* getOutputForChannel(int channel) override { return channel == 0 ? &mPanner->leftOutput : &mPanner->rightOutput; }
            int getChannelCount() const override { return 2; }
            void connect(unsigned int channel, OutputPin& pin) { mPanner->audioInput.connect(pin); }

            StereoGrainPannerNode& getPanner() { return *mPanner; }

        private:
            SafeOwner<StereoGrainPannerNode> mPanner = nullptr;
        };



        class NAPAPI Granulator : public AudioObject
        {
        public:
            Granulator() = default;

            ResourcePtr<AudioBufferResource> mBuffer = nullptr;
            int mInternalBufferSize = 64;
            int mVoiceCount = 100;

        private:
            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;
        };



        class NAPAPI GranulatorInstance : public AudioObjectInstance
        {
        public:
            /**
            * Possible amplitude envelope shapes for a grain
            */
            enum Shape { hanning, expodec, rexpodec };

        public:
            GranulatorInstance() = default;
            GranulatorInstance(const std::string& name) : AudioObjectInstance(name) { }

            /**
            * Initialize the granulator channel
            * @param channelIndex the index of the channel
            * @param internalBufferSize the internal buffersize of the granulator DSP
            * @param voiceCount the number of voices of the granulator per channel
            * @param voice resource for the DSP graph of a single voice
            * @param circularBuffer the circular buffer where the audio
            * @param pitchTranslator table that will be used to convert semitones difference to pitch factor
            * @param nodeManager main node manager of the DSP system
            * @param errorState error logging
            * @return true on success
            */
            bool init(int internalBufferSize, int voiceCount, SafePtr<CircularBufferNode> circularBuffer, NodeManager& nodeManager, utility::ErrorState& errorState);

            bool init(int internalBufferSize, int voiceCount, SafePtr<MultiSampleBuffer> buffer, NodeManager& nodeManager, utility::ErrorState& errorState);

            /**
            * Set the timing deviation in ms of the grains measured in whole times the average interval between the grains
            */
            void setIrregularity(float value) { mIrregularity = value; }

            /**
            * Set the deviation in the read position from the circular buffer in ms
            */
            void setDiffusion(float value) { mDiffusion = value; }

            /**
            * Set the shape of the amplitude envelope of the grains.
            */
            void setShape(GranulatorInstance::Shape shape) { mShape = shape; }

            /**
            * Sets the attack for expodec shaped grains/the decay of the rexpodec grains
            */
            void setAttackDecay(TimeValue value) { mAttackDecay = value; }

            /**
            * Sets the deviation of the transpose value in semitones
            */
            void setTransposeDeviation(float value) { mTransposeDeviation = value; }

            /**
            * Sets the average amplitude of the grains
            */
            void setAmplitude(float value) { mAmplitude = value; }

            /**
            * Sets the deviation of the amplitudes of the grains
            */
            void setAmplitudeDev(float value) { mAmplitudeDev = value; }

            /**
            * Sets wether the granulator is activated.
            */
            void setActive(bool active);

            /**
            * Sets the average transposition of the pitch in semitones.
            */
            void setTranspose(float value);

            /**
            * Sets the duration of the grains in ms.
            */
            void setDuration(TimeValue value);

            /**
            * Sets the density of the granulator: number of grains
            */
            void setDensity(float value);

            /**
             * Sets the read position within the memory buffer
             */
            void setPosition(double time);

            void setSpeed(float value) { mSpeed = value; }

            /**
            * Audio output signal
            */
            OutputPin* getOutput();

            /**
             * Change the source from which audio input is read to a circular buffer and set the mode accordingly to streaming input
             */
            void setSource(SafePtr<CircularBufferNode> buffer);

            /**
             * Change the buffer from which audio input is read to a memory buffer and set the mode accordingly
             */
            void setSource(SafePtr<MultiSampleBuffer> buffer);

        private:
            std::unique_ptr<NestedNodeManagerInstance> mNestedNodeManager = nullptr;
            std::unique_ptr<PolyphonicInstance> mPolyphonic = nullptr;
            std::unique_ptr<NodeObjectInstance<OutputNode>> mOutput = nullptr;

            SafePtr<CircularBufferNode> mCircularBuffer = nullptr;
            SafePtr<MultiSampleBuffer> mBuffer = nullptr;

            SafeOwner<TableTranslator<float>> mPitchTranslator = nullptr;

            float mAmplitudeScaling = 1.f;
            bool mActive = false;
            TimeValue mDuration = 500;
            float mIrregularity = 0;
            float mDiffusion = 0;
            float mDensity = 10;
            float mAmplitude = 1;
            float mAmplitudeDev = 0;
            Shape mShape = hanning;
            TimeValue mAttackDecay = 10;
            float mTranspose = 1.f;
            float mTransposeDeviation = 0;


            // Specific for memory buffer granulation
            double mPosition = 0; // in milliseconds
            float mSpeed = 1.f;

        private:
            void updateAudio(DiscreteTimeValue sampleTime);
            void makeGrain();
            bool init(int internalBufferSize, int voiceCount, NodeManager& nodeManager, utility::ErrorState& errorState);

            double mCurrentPhase = 0;
            DiscreteTimeValue mLastSampleTime = 0;
            bool mAboutToSpawn = false;
            double mIrregularityOffset = 0.;
        };

    }

}