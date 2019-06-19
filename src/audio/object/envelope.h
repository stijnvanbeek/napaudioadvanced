#pragma once

// Nap includes
#include <rtti/objectptr.h>

// Audio includes
#include <audio/core/audioobject.h>
#include <audio/node/envelopenode.h>
#include <audio/utility/translator.h>

namespace nap
{

    namespace audio
    {

        class EnvelopeInstance;

        /**
         * Audio object resource for an envelope generator.
         */
        class NAPAPI Envelope : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
        public:
            Envelope() = default;

            EnvelopeGenerator::Envelope mSegments; ///< The segments that define the envelope's shape.
            bool mAutoTrigger = false; ///< If true the envelope will be triggered automatically on initialization.
            bool mEqualPowerTranslate = false; ///< Indicated wether the output will be translated using an equal power table.

        private:
            // Inherited from AudioObject
            std::unique_ptr<AudioObjectInstance> createInstance(AudioService& audioService, utility::ErrorState& errorState) override;
        };


        /**
         * Instance of an envelope generator.
         */
        class NAPAPI EnvelopeInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
        public:
            EnvelopeInstance() : AudioObjectInstance() { }
            EnvelopeInstance(const std::string& name) : AudioObjectInstance(name) { }

            // Inherited from AudioObjectInstance
            bool init(EnvelopeGenerator::Envelope segments, bool autoTrigger, AudioService& audioService, utility::ErrorState& errorState);
            
            OutputPin* getOutputForChannel(int channel) override { return &mEnvelopeGenerator->output; }
            int getChannelCount() const override { return 1; }

            /**
             * Triggers the envelope to start playing from the start segment.
             * If @totalDuration does not equal zero the relative durations in the segments will be scaled in order to get the total duration of the envelope to match this parameter.
             */
            void trigger(TimeValue totalDuration = 0)
            {
                mEnvelopeGenerator->trigger(totalDuration);
            }

            /**
             * Stops playing the envelope by fading to zero within @rampTime.
             */
            void stop(TimeValue rampTime) { mEnvelopeGenerator->stop(rampTime); }

            /**
             * Sets the envelope data for one segment of the envelope. The segment index specifies which segment will be edited.
             * If the index out of bounds no action will be taken.
             */
            void setSegmentData(unsigned int segmentIndex, TimeValue duration, ControllerValue destination, bool durationRelative, bool exponential, bool useTranslator);

            /**
             * Returns the current output value of the envelope generator.
             */
            ControllerValue getValue() const { return mEnvelopeGenerator->getValue(); }

            /**
             * Returns a signal that will be emitted when the total envelope shape has finished and the generator outputs zero again.
             */
            nap::Signal<EnvelopeGenerator&>& getEnvelopeFinishedSignal() { return mEnvelopeGenerator->envelopeFinishedSignal; }

            /**
             * Returns a sginal that will be emitted when one segment of the envelope has finished playing. The semgent index of the ENvelopeGenerator still contains the number of the segment that has just finished.
             */
            nap::Signal<EnvelopeGenerator&>& getSegmentFinishedSignal() { return mEnvelopeGenerator->segmentFinishedSignal; }
            
        private:
            SafeOwner<EnvelopeGenerator> mEnvelopeGenerator = nullptr;
            SafeOwner<EqualPowerTranslator<ControllerValue>> mEqualPowerTable = nullptr;
        };


    }

}
