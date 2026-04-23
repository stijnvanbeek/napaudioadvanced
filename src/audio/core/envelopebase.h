#pragma once

#include <audio/core/audioobject.h>

namespace nap
{
    namespace audio
    {

        // Forward declaration
        class EnvelopeInstanceBase;


        /**
         * This is an AudioObject resource placeholder for envelope instances.
         * All descendents of EnvelopBase should create instances derived from EnvelopeInstanceBase.
         */
        class NAPAPI EnvelopeBase : public AudioObject
        {
            RTTI_ENABLE(AudioObject)

        public:
            EnvelopeBase() = default;
        };


        /**
         * Base class exposing interface to respond to envelope events.
         * Used by PolyphonicInstance to respond to voice envelopes finishing by disconnecting the voice.
         */
        class NAPAPI EnvelopeInstanceBase : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)

        public:
            EnvelopeInstanceBase() = default;
            EnvelopeInstanceBase(const std::string& name) : AudioObjectInstance(name) { }

            /**
             * Should trigger playing the envelope
             * @param duration Total duration of the envelope. The argument is a suggestion, the implementation is free to differ.
             */
            virtual void trigger(TimeValue duration) = 0;

            /**
             * Triggers a subsection of the envelope.
             * @param startSegment
             * @param endSegment
             * @param startValue
             * @param endValue
             */
            virtual void triggerSection(int startSegment, int endSegment, ControllerValue startValue, ControllerValue endValue) = 0;

            /**
             * Breaks playback of the envelope and fades out.
             * @param rampTime Fade out time in ms.
             */
            virtual void stop(TimeValue rampTime) = 0;

            /**
             * @return The current output value of the envelope
             */
            virtual ControllerValue getValue() const = 0;

            /**
             * @return A signal that will be emitted when the total envelope shape has finished and the generator outputs zero again.
             */
            virtual nap::Signal<>& getEnvelopeFinishedSignal() = 0;

            /**
             * @return A sginal that will be emitted when one segment of the envelope has finished playing.
             * The signal passes the index of the segment that has just finished playing.
             */
            virtual nap::Signal<int>& getSegmentFinishedSignal() = 0;
        };
    
    }
}