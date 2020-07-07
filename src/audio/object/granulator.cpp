#include "granulator.h"

#include <audio/object/circularbufferplayer.h>
#include <audio/object/bufferplayer.h>

RTTI_BEGIN_ENUM(nap::audio::Granulator::Shape)
    RTTI_ENUM_VALUE(nap::audio::Granulator::Shape::hanning, "Hanning"),
    RTTI_ENUM_VALUE(nap::audio::Granulator::Shape::expodec, "Expodec"),
    RTTI_ENUM_VALUE(nap::audio::Granulator::Shape::rexpodec, "Rexpodec")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::audio::Granulator)
    RTTI_PROPERTY("Buffer", &nap::audio::Granulator::mBuffer, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("InternalBufferSize", &nap::audio::Granulator::mInternalBufferSize, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("VoiceCount", &nap::audio::Granulator::mVoiceCount, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Active", &nap::audio::Granulator::mActive, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Duration", &nap::audio::Granulator::mDuration, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Irregularity", &nap::audio::Granulator::mIrregularity, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Diffusion", &nap::audio::Granulator::mDiffusion, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Density", &nap::audio::Granulator::mDensity, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Amplitude", &nap::audio::Granulator::mAmplitude, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("AmplitudeDev", &nap::audio::Granulator::mAmplitudeDev, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Shape", &nap::audio::Granulator::mShape, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("AttackDecay", &nap::audio::Granulator::mAttackDecay, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Transpose", &nap::audio::Granulator::mTranspose, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("TransposeDeviation", &nap::audio::Granulator::mTransposeDeviation, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Position", &nap::audio::Granulator::mPosition, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Speed", &nap::audio::Granulator::mSpeed, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


namespace nap
{

    namespace audio
    {


        void StereoGrainPannerNode::setPanning(ControllerValue panning, ControllerValue amplitude, TimeValue maxDelay)
        {
            assert(panning >= 0.f);
            assert(panning <= 1.f);

            equalPowerPan(panning, leftGain, rightGain);
            leftGain *= amplitude;
            rightGain *= amplitude;

            mDelay.clear();
            if (panning < 0.5f)
            {
                TimeValue mDelayTime = (1.f - panning * 2) * maxDelay;
                leftDelayTime = mDelayTime * getNodeManager().getSamplesPerMillisecond();
                rightDelayTime = 0.f;
            }
            else {
                TimeValue mDelayTime = (2 * panning - 1.f) * maxDelay;
                rightDelayTime = mDelayTime * getNodeManager().getSamplesPerMillisecond();
                leftDelayTime = 0.f;
            }
        }


        void StereoGrainPannerNode::process()
        {
            auto inputBuffer = audioInput.pull();
            auto envelopeBuffer = envelopeInput.pull();
            assert(inputBuffer != nullptr);
            assert(envelopeBuffer != nullptr);

            auto& leftOutputBuffer = getOutputBuffer(leftOutput);
            auto& rightOutputBuffer = getOutputBuffer(rightOutput);

            for (auto i = 0; i < inputBuffer->size(); ++i)
            {
                mDelay.write((*inputBuffer)[i]  * (*envelopeBuffer)[i]);
                leftOutputBuffer[i] = mDelay.read(leftDelayTime) * leftGain;
                rightOutputBuffer[i] = mDelay.read(rightDelayTime) * rightGain;
            }
        }


        std::unique_ptr<AudioObjectInstance> StereoGrainPanner::createInstance(NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            auto result = std::make_unique<StereoGrainPannerInstance>(mID);
            if (result->init(*this, nodeManager, errorState))
                return result;
            else
                return nullptr;
        }


        bool StereoGrainPannerInstance::init(StereoGrainPanner& resource, NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            mPanner = nodeManager.makeSafe<StereoGrainPannerNode>(nodeManager);
            if (resource.mInput != nullptr)
            {
                if (resource.mInput->getInstance()->getChannelCount() < 1)
                {
                    errorState.fail("StereoGrainPanner input has no output channels");
                    return false;
                }
                mPanner->audioInput.connect(*resource.mInput->getInstance()->getOutputForChannel(0));
            }

            if (resource.mEnvelope != nullptr)
                mPanner->envelopeInput.connect(*resource.mEnvelope->getInstance()->getOutputForChannel(0));

            return true;
        }


        std::unique_ptr<AudioObjectInstance> Granulator::createInstance(NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            auto result = std::make_unique<GranulatorInstance>(mID);
            if (!result->init(mInternalBufferSize, mVoiceCount, mBuffer->getBuffer(), mBuffer->getSampleRate(), nodeManager, errorState))
            {
                errorState.fail("Failed to initialize granulator");
                return nullptr;
            }

            result->setDuration(mDuration);
            result->setIrregularity(mIrregularity);
            result->setDiffusion(mDiffusion);
            result->setDensity(mDensity);
            result->setAmplitude(mAmplitude);
            result->setAmplitudeDev(mAmplitudeDev);
            result->setShape(mShape);
            result->setAttackDecay(mAttackDecay);
            result->setTranspose(mTranspose);
            result->setTransposeDeviation(mTransposeDeviation);
            result->setPosition(mPosition);
            result->setSpeed(mSpeed);
            result->setActive(mActive);

            return result;
        }


        GranulatorInstance::~GranulatorInstance()
        {
            AsyncObserver observer;
            auto observerPtr = &observer;

            mNestedNodeManager->getNestedNodeManager().enqueueTask([&, observerPtr](){
               mNestedNodeManager->getNestedNodeManager().mUpdateSignal.disconnect(mUpdateAudioSlot);
               observerPtr->notifyOne();
            });

            observer.waitForNotifications();
        }


        bool GranulatorInstance::init(int internalBufferSize, int voiceCount, SafePtr<CircularBufferNode> circularBuffer, NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            mCircularBuffer = circularBuffer;
            return init(internalBufferSize, voiceCount, nodeManager, errorState);
        }


        bool GranulatorInstance::init(int internalBufferSize, int voiceCount, SafePtr<MultiSampleBuffer> buffer, float bufferSampleRate, NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            mBuffer = buffer;
            mBufferSampleRate = bufferSampleRate;
            return init(internalBufferSize, voiceCount, nodeManager, errorState);
        }


        void GranulatorInstance::updateAudio(DiscreteTimeValue sampleTime)
        {

            if (mActive)
            {
                if (sampleTime >= mNextGrainTime)
                {
                    makeGrain(sampleTime);
                    mLastGrainTime = sampleTime;
                    auto interval = (mDuration / mDensity) * (0.5f + mIrregularity * math::random(0.f, 1.f));
                    mNextGrainTime = sampleTime + interval * mNestedNodeManager->getNestedNodeManager().getSamplesPerMillisecond();
                }

            }

            mLastSampleTime = sampleTime;
        }


        void GranulatorInstance::makeGrain(DiscreteTimeValue sampleTime)
        {
            auto voice = mPolyphonic->findFreeVoice();
            if (voice == nullptr)
                return;
            auto& panner = voice->getObject<StereoGrainPannerInstance>("panner")->getPanner();
            auto envelope = voice->getObject<EnvelopeInstance>("envelope");

            auto transpose = mTranspose + (-1 + 2 * math::random(0.f, 1.f)) * mTransposeDeviation;
            auto pitch = mPitchTranslator->translate((transpose + 12) / 24.f);

            auto amplitude = mAmplitude * mAmplitudeScaling;
            auto amplitudeNoisy = amplitude * (1 + (math::random(0.f, 1.f) * mAmplitudeDev * 2) - mAmplitudeDev);

            panner.setPanning(math::random(0.f, 1.f), amplitudeNoisy, 30.f);

            switch (mShape)
            {
                case Granulator::Shape::hanning:
                {
                    envelope->setSegmentData(0, mDuration * 0.5, 1.0, false, false, true);
                    envelope->setSegmentData(1, mDuration * 0.5, 0.0, false, false, true);
                    envelope->setSegmentData(2, panner.getDelayTime(), 0.0, false, false, true);
                    break;
                }
                case Granulator::Shape::expodec:
                {
                    float limitedAttackDecay = 0.f;
                    if (mAttackDecay > mDuration * 0.5)
                        limitedAttackDecay = mDuration * 0.5;
                    else {
                        limitedAttackDecay = mAttackDecay;
                        envelope->setSegmentData(0, limitedAttackDecay, 1.0, false, false, false);
                        envelope->setSegmentData(1, mDuration - limitedAttackDecay, 0.0, false, true, false);
                        envelope->setSegmentData(2, panner.getDelayTime(), 0.0, false, false, true);
                    }
                    break;
                }
                case Granulator::Shape::rexpodec:
                {
                    float limitedAttackDecay = 0.f;
                    if (mAttackDecay > mDuration * 0.5)
                        limitedAttackDecay = mDuration * 0.5;
                    else {
                        limitedAttackDecay = mAttackDecay;
                        envelope->setSegmentData(0, mDuration - limitedAttackDecay, 1.0, false, true, false);
                        envelope->setSegmentData(1, limitedAttackDecay, 0.0, false, false, false);
                        envelope->setSegmentData(2, panner.getDelayTime(), 0.0, false, false, true);
                    }
                    break;
                }
            }

            if (mCircularBuffer != nullptr)
            {
                auto samplesPerMillisecond = mCircularBuffer->getSampleRate() / 1000.0;
                DiscreteTimeValue pos = 0;
                if (pitch > 1)
                    pos = int((mDuration * (pitch - 1) + mDiffusion * math::random(0.f, 1.f)) * samplesPerMillisecond) + mCircularBuffer->getBufferSize() * 2;
                else
                    pos = int(mDiffusion * math::random(0.f, 1.f) * samplesPerMillisecond) + mCircularBuffer->getBufferSize() * 2;
                // fixme: why does it need bufferSize * 2?

                auto circularBufferPlayer = voice->getObject<ParallelNodeObjectInstance<CircularBufferPlayerNode>>("circularBufferPlayer")->getChannel(0);
                circularBufferPlayer->play(*mCircularBuffer, pos, pitch);
                panner.audioInput.connect(circularBufferPlayer->audioOutput);
            }

            else if (mBuffer != nullptr)
            {
                auto samplesPerMillisecond = mBufferSampleRate / 1000.f;
                auto posInc = mSpeed * (sampleTime - mLastGrainTime) / mNestedNodeManager->getNestedNodeManager().getSamplesPerMillisecond();
                mPosition += posInc;
                DiscreteTimeValue pos = int((mPosition + mDiffusion * math::random(0.f, 1.f)) * samplesPerMillisecond);

                auto bufferPlayer = voice->getObject<ParallelNodeObjectInstance<BufferPlayerNode>>("bufferPlayer")->getChannel(0);
                bufferPlayer->stop();
                bufferPlayer->setBuffer(mBuffer);
                bufferPlayer->play(0, pos, pitch);
                panner.audioInput.connect(bufferPlayer->audioOutput);
            }

            mPolyphonic->play(voice, 0);
        }


        bool GranulatorInstance::init(int internalBufferSize, int voiceCount, NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            mPitchTranslator = nodeManager.makeSafe<audio::TableTranslator<float>>(2048);
            mPitchTranslator->fill([](float input)
                                   {
                                       return std::pow(2, (-12 + input * 24) / 12.f);
                                   });

            auto mEqualPowerTable = std::make_unique<EqualPowerTable>(nodeManager);
            mEqualPowerTable->mID = "equalPowerTable";
            mEqualPowerTable->mSize = 128;
            if (!mEqualPowerTable->init(errorState))
            {
                errorState.fail("Failed to initialize equal power table");
                return false;
            }

            Envelope envelope;
            envelope.mID = "envelope";

            EnvelopeNode::Segment attack;
            attack.mDuration = 5.f;
            attack.mDestination = 1.f;
            attack.mDurationRelative = false;
            attack.mMode = RampMode::Linear;
            attack.mTranslate = true;

            EnvelopeNode::Segment decay;
            decay.mDuration = 200.f;
            decay.mDestination = 0.f;
            decay.mDurationRelative = false;
            decay.mMode = RampMode::Linear;
            decay.mTranslate = true;

            EnvelopeNode::Segment delay;
            decay.mDuration = 50.f;
            decay.mDestination = 0.f;
            decay.mDurationRelative = false;
            decay.mMode = RampMode::Linear;
            decay.mTranslate = false;

            envelope.mSegments.emplace_back(attack);
            envelope.mSegments.emplace_back(decay);

            envelope.mEqualPowerTable = mEqualPowerTable.get();

            CircularBufferPlayer circularBufferPlayer;
            circularBufferPlayer.mID = "circularBufferPlayer";
            circularBufferPlayer.mChannelCount = 1;

            BufferPlayer bufferPlayer;
            bufferPlayer.mID = "bufferPlayer";
            bufferPlayer.mChannelCount = 1;
            bufferPlayer.mAutoPlay = false;

            StereoGrainPanner panner;
            panner.mID = "panner";
            panner.mEnvelope = &envelope;

            Voice voice;
            voice.mObjects.emplace_back(&envelope);
            voice.mObjects.emplace_back(&circularBufferPlayer);
            voice.mObjects.emplace_back(&bufferPlayer);
            voice.mObjects.emplace_back(&panner);
            voice.mOutput = &panner;
            voice.mEnvelope = &envelope;

            mNestedNodeManager = std::make_unique<NestedNodeManagerInstance>();
            if (!mNestedNodeManager->init(nodeManager, 0, 2, internalBufferSize, errorState))
            {
                errorState.fail("Failed to initialize nested node manager.");
                return false;
            }

            mPolyphonic = std::make_unique<PolyphonicInstance>();
            if (!mPolyphonic->init(voice, voiceCount, true, 2, mNestedNodeManager->getNestedNodeManager(), errorState))
            {
                errorState.fail("Failed to initialize Polyphonic");
                return false;
            }

            mOutput = std::make_unique<ParallelNodeObjectInstance<OutputNode>>();
            if (!mOutput->init(2, mNestedNodeManager->getNestedNodeManager(), errorState))
            {
                errorState.fail("Failed to create nested nodemanager's output");
                return false;
            }
            mOutput->getChannel(0)->setOutputChannel(0);
            mOutput->getChannel(1)->setOutputChannel(1);

            mOutput->AudioObjectInstance::connect(*mPolyphonic);
            mNestedNodeManager->getNestedNodeManager().registerRootProcess(*mOutput->getChannel(0));
            mNestedNodeManager->getNestedNodeManager().registerRootProcess(*mOutput->getChannel(1));

            mNestedNodeManager->getNestedNodeManager().mUpdateSignal.connect(mUpdateAudioSlot);

            return true;
        }



        void GranulatorInstance::setActive(bool active)
        {
            if (active == mActive)
                return;

            mActive = active;
        }


        void GranulatorInstance::setTranspose(float value)
        {
            mTranspose = value;
        }


        void GranulatorInstance::setDuration(TimeValue value)
        {
            mDuration = value;
        }


        void GranulatorInstance::setDensity(float value)
        {
            mDensity = value;
        }


        void GranulatorInstance::setPosition(double time)
        {
            mPosition = time;
        }


        OutputPin* GranulatorInstance::getOutput()
        {
            return mNestedNodeManager->getOutputForChannel(0);
        }


        void GranulatorInstance::setSource(SafePtr<CircularBufferNode> buffer)
        {
            mCircularBuffer = buffer;
            mBuffer = nullptr;
        }


        void GranulatorInstance::setSource(SafePtr<MultiSampleBuffer> buffer, float sampleRate)
        {
            mBuffer = buffer;
            mBufferSampleRate = sampleRate;
            mCircularBuffer = nullptr;
        }

    }

}
