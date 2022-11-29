/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <atomic>

#include <audio/core/graph.h>
#include <audio/object/envelope.h>


namespace nap
{
    
    namespace audio
    {
        
        // Forward declarations
        class PolyphonicInstance;
        
        
        /**
         * A specific type of graph that is used to define a DSP network for one voice of processing in a polyphonic system.
         */
        class Voice : public Graph
        {
            RTTI_ENABLE(Graph)
            
        public:
            Voice() : Graph()  { }
            
            ResourcePtr<Envelope> mEnvelope = nullptr; ///< Property: 'Envelope' Points to an envelope within the graph that controls the amplitude of a single audio event processed by the voice. When the voice is played this envelope will be triggered. When it has finished it emits a signal that will cause the voice to be disconnected and enter idle state again.
            
        private:
        };
        
        
        /**
         * Instance of a Voice graph representing one voice within a polyphonic system.
         */
        class VoiceInstance : public GraphInstance
        {
            RTTI_ENABLE(GraphInstance)
            
            friend class PolyphonicInstance;
            
        public:
            bool init(Voice& resource, NodeManager& nodeManager, utility::ErrorState& errorState);
            
            /**
             * @return The envelope controlling the overall amplitude of the voice
             */
            EnvelopeInstance& getEnvelope() { return *mEnvelope; }
            
            /**
             * @return The envelope controlling the overall amplitude of the voice
             */
            const EnvelopeInstance& getEnvelope() const { return *mEnvelope; }

            /**
             * Starts playback of the voice by triggering the envelope
             * @param duration The total duration of the envelope. This parameter will only have effect if the voice's envelope data has segments with a relative duration. See Envelope for more info.
             */
            void play(TimeValue duration = 0);

            /**
             * Starts playback of the voice by triggering a section of the envelope
             * @param startSegment The index of the starting segment of the voice envelope's subsection that will be triggered.
             * @param endSegmtnt The index of the ending segment of the voice envelope's subsection that will be triggered.
             * @param startValue As envelope segments only define their destination value, this parameter can be used to define the starting value of the first segment. Normally this will be 0 to avoid clicks and pops.
             * @param totalDuration The total duration of the envelope's subsection. This parameter will only have effect if the voice's envelope data has segments with a relative duration. See Envelope for more info.
             */
            void playSection(int startSegment, int endSegment, ControllerValue startValue = 0, TimeValue totalDuration = 0);

            /**
             * Stops playback of the voice by forcing the envelope to fade out
             * @param fadeOutTime The fadeout time in ms from the moment this method is called.
             */
            void stop(TimeValue rampTime);
            
            /**
             * @return True if this voice is currently playing or reserved for usage.
             */
            bool isBusy() const { return mBusy; }
            
            /**
             * @return When the voice is busy, the time the voice started playing
             */
            DiscreteTimeValue getStartTime() const { return mStartTime; }

            /**
             * Signal that is emitted when the voice has finished playing.
             */
            nap::Signal<VoiceInstance&> finishedSignal;
            
            /**
             * Function to retrieve the @finishedSignal using RTTR
             */
            nap::Signal<VoiceInstance&>* getFinishedSignal() { return &finishedSignal; }
            
        private:
            // Used internally by PolyphincObjectInstance to try to reserve the voice for usage
            bool try_use();
            void free();
            
            // Responds to the signal emitted by the envelope generator of the main envelope by emitting the finishedSignal.
            Slot<EnvelopeNode&> envelopeFinishedSlot = {this, &VoiceInstance::envelopeFinished };
            void envelopeFinished(EnvelopeNode&);

            EnvelopeInstance* mEnvelope = nullptr;
            std::atomic<bool> mBusy = { false };
            DiscreteTimeValue mStartTime = 0;
            
            // This set caches the channels of the output mixer of the polyphonic object that this voice is connected to before it was started to play. When playing is done the polyphonic object will take care of disconnecting the voice from these channels.
            std::vector<int> mConnectedToChannels = { };
        };
        
    }
    
}
