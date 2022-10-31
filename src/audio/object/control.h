/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <audio/core/audioobject.h>
#include <audio/node/controlnode.h>
#include <audio/resource/equalpowertable.h>
#include <nap/resourceptr.h>

namespace nap
{

	namespace audio
	{

		class ControlInstance;

		/**
		 * Audio object resource for an envelope generator.
		 */
		class NAPAPI Control : public AudioObject
		{
			RTTI_ENABLE(AudioObject)

		public:
			Control() = default;

			ControllerValue mValue = 0.f; ///< property: 'Value' Initial output value
			ResourcePtr<EqualPowerTable> mEqualPowerTable = nullptr; ///< Property: 'EqualPowerTable' used to translate to equal power curve.

		private:
			// Inherited from AudioObject
			std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;
		};


		/**
		 * Instance of an envelope generator.
		 */
		class NAPAPI ControlInstance : public AudioObjectInstance
		{
			RTTI_ENABLE(AudioObjectInstance)

		public:
			ControlInstance() : AudioObjectInstance() { }
			ControlInstance(const std::string& name) : AudioObjectInstance(name) { }

			// Inherited from AudioObjectInstance
			bool init(NodeManager& nodeManager, audio::SafePtr<Translator<float>>& translator, utility::ErrorState& errorState);

			OutputPin* getOutputForChannel(int channel) override { return &mControlNode->output; }
			int getChannelCount() const override { return 1; }

			/**
			 * Set the output value immediately.
			 */
			void setValue(ControllerValue value) { mControlNode->setValue(value); }

			/**
			 * Return the output value, optionally shaped by the lookup translator.
			 */
			ControllerValue getValue() const { return mControlNode->getValue(); }

			/**
			 * Return the output value bypassing the loopkup translator.
			 */
			ControllerValue getRawValue() const { return mControlNode->getRawValue(); }

			/**
			 * Start ramping to @destination over a period of @time, using mode to indicate the type of ramp.
			 */
			void ramp(ControllerValue destination, TimeValue time, RampMode mode = RampMode::Linear) { mControlNode->ramp(destination, time, mode); }

			/**
			 * Linear call to ramp() to expose to rttr for python scripting purposes.
			 */
			void rampLinear(ControllerValue destination, TimeValue time) { ramp(destination, time); }

			/**
			 * @return: wether the object is currently ramping to a new value.
			 */
			bool isRamping() const { return mControlNode->isRamping(); }

			/**
			 * Stops the current ramp (if any) and stays on the current value.
			 */
			void stop() { mControlNode->stop(); }

			/**
			 * Assign a translator to this node to shape the output value.
			 */
			void setTranslator(SafePtr<Translator<ControllerValue>>& translator) { mControlNode->setTranslator(translator); }

			/**
			 * @return: wether this node uses a translator lookup table to shape it's output values.
			 */
			bool hasTranslator() const { return mControlNode->hasTranslator(); }

			/**
			 * Returns a sginal that will be emitted when a ramp has finished.
			 */
			nap::Signal<ControlNode&>& getRampFinishedSignal() { return mControlNode->rampFinishedSignal; }

		private:
			SafeOwner<ControlNode> mControlNode = nullptr;
		};


	}

}