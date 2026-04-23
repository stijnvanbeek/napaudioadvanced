/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Nap includes
#include <rtti/objectptr.h>
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/envelopebase.h>
#include <audio/node/envelopenode.h>
#include <audio/resource/equalpowertable.h>

namespace nap
{

    namespace audio
    {

        class EnvelopeInstance;

        /**
         * Audio object resource for an envelope generator.
         */
        class NAPAPI Envelope : public EnvelopeBase
        {
            RTTI_ENABLE(AudioObject)
        public:
            Envelope() = default;

            EnvelopeNode::Envelope mSegments; ///< Property: 'Segments' The segments that define the envelope's shape.
            bool mAutoTrigger = false;        ///< Property: 'AutoTrigger' If true the envelope will be triggered automatically on initialization.
            ResourcePtr<EqualPowerTable> mEqualPowerTable = nullptr; ///< Property: 'EqualPowerTable' used to translate to equal power curve.

        private:
            // Inherited from AudioObject
            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;
        };


        /**
         * Instance of an envelope generator.
         */
        class NAPAPI EnvelopeInstance : public EnvelopeInstanceBase
        {
            RTTI_ENABLE(EnvelopeInstanceBase)
        public:
            EnvelopeInstance() : EnvelopeInstanceBase() { }
            EnvelopeInstance(const std::string& name) : EnvelopeInstanceBase(name) { }

            // Inherited from AudioObjectInstance
            bool init(EnvelopeNode::Envelope segments, bool autoTrigger, NodeManager& nodeManager, audio::SafePtr<Translator<float>> translator, utility::ErrorState& errorState);
            
            OutputPin* getOutputForChannel(int channel) override { return &mEnvelopeGenerator->output; }
            int getChannelCount() const override { return 1; }

            // Inherited from EnvelopeInstanceBase
            nap::Signal<>& getEnvelopeFinishedSignal() override { return mEnvelopeGenerator->envelopeFinishedSignal; }
            nap::Signal<int>& getSegmentFinishedSignal() override { return mEnvelopeGenerator->segmentFinishedSignal; }

            /**
             * Triggers the envelope to start playing from the start segment.
             * If @totalDuration does not equal zero the relative durations in the segments will be scaled in order to get the total duration of the envelope to match this parameter.
             */
            void trigger(TimeValue totalDuration = 0) override
            {
                mEnvelopeGenerator->trigger(totalDuration);
            }
            
            /**
             * Triggers a section of an envelope.
             * @param totalDuration: if this value is greater than the total of all durations of segments that have durationRelative = false
             * @param startSegment: the start segment of the envelope section to be played
             * @param endSegment: the end segment of the envelope section to be played
             * @param startValue: the startValue of the line when the section is triggered.
             * @param totalDuration: if this value is greater than the total of all durations of segments that have durationRelative = false
             the resting time wille be divided over the segments with durationRelative = true, using their duration values as denominator.
             */
            void triggerSection(int startSegment, int endSegment, ControllerValue startValue = 0, TimeValue totalDuration = 0) override
            {
                mEnvelopeGenerator->trigger(startSegment, endSegment, startValue, totalDuration);
            }

            /**
             * Stops playing the envelope
             * @param rampTime fade out time in ms
             */
            void stop(TimeValue rampTime) override { mEnvelopeGenerator->stop(rampTime); }

            /**
             * @return the current output value of the envelope generator.
             */
            ControllerValue getValue() const override { return mEnvelopeGenerator->getValue(); }

            /**
              * Sets the envelope data for one segment of the envelope.
              * @param segmentIndex Specifies which segment will be edited.
              * If the index out of bounds no action will be taken.
              * @param duration New duration of the segment in ms or as a relative fraction of the total duration of the envelope
              * @param destination New destination value of the segment
              * @param durationRelative True if the duration of the segment is a relative fraction of the total duration of the envelope
              * @param exponential True if the curvature of the segment is exponential, false if the segment is linear
              * @param useTranslator True if the translator object should be used to translate the output value
              */
            void setSegmentData(unsigned int segmentIndex, TimeValue duration, ControllerValue destination, bool durationRelative, bool exponential, bool useTranslator);
            
            /**
             * Assigns new envelope data
             * @param envelope Input envelope data that will be copied to this object
             */
            void setEnvelopeData(const EnvelopeNode::Envelope& envelope) { mEnvelopeGenerator->getEnvelope() = envelope; }

        private:
            SafeOwner<EnvelopeNode> mEnvelopeGenerator = nullptr;
            SafePtr<Translator<ControllerValue>> mTranslator = nullptr;
        };


    }

}
