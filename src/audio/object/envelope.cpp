#include "envelope.h"

RTTI_BEGIN_CLASS(nap::audio::Envelope)
    RTTI_PROPERTY("Envelope", &nap::audio::Envelope::mSegments, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("AutoTrigger", &nap::audio::Envelope::mAutoTrigger, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("EqualPowerTranslate", &nap::audio::Envelope::mEqualPowerTranslate, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::EnvelopeInstance)
    RTTI_FUNCTION("trigger", &nap::audio::EnvelopeInstance::trigger)
    RTTI_FUNCTION("stop", &nap::audio::EnvelopeInstance::stop)
    RTTI_FUNCTION("setSegmentData", &nap::audio::EnvelopeInstance::setSegmentData)
RTTI_END_CLASS

namespace nap
{

    namespace audio
    {

        std::unique_ptr<AudioObjectInstance> Envelope::createInstance(AudioService& audioService, utility::ErrorState& errorState)
        {
            auto instance = std::make_unique<EnvelopeInstance>();
            if (!instance->init(mSegments, mAutoTrigger, audioService, errorState))
                return nullptr;
            
            return instance;
        }


        bool EnvelopeInstance::init(EnvelopeGenerator::Envelope segments, bool autoTrigger, AudioService& audioService, utility::ErrorState& errorState)
        {
            mEqualPowerTable = audioService.makeSafe<EqualPowerTranslator<ControllerValue>>(256);
            mEnvelopeGenerator = audioService.makeSafe<EnvelopeGenerator>(audioService.getNodeManager(), segments, mEqualPowerTable.get());

            if (autoTrigger)
                mEnvelopeGenerator->trigger();

            return true;
        }


        void EnvelopeInstance::setSegmentData(unsigned int segmentIndex, TimeValue duration, ControllerValue destination, bool durationRelative, bool exponential, bool useTranslator)
        {
            if (segmentIndex >= mEnvelopeGenerator->getEnvelope().size())
                return;
            
            auto& segment = mEnvelopeGenerator->getEnvelope()[segmentIndex];
            segment.mDuration = duration;
            segment.mDestination = destination;
            segment.mDurationRelative = durationRelative;
            segment.mMode = exponential ? RampMode::Exponential : RampMode::Linear;
            segment.mTranslate = useTranslator;
        }

    }

}
