/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "control.h"

RTTI_BEGIN_CLASS(nap::audio::Control)
	RTTI_PROPERTY("Value", &nap::audio::Control::mValue, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("EqualPowerTable", &nap::audio::Control::mEqualPowerTable, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::ControlInstance)
	RTTI_FUNCTION("setValue", &nap::audio::ControlInstance::setValue)
	RTTI_FUNCTION("ramp", &nap::audio::ControlInstance::rampLinear)
	RTTI_FUNCTION("stop", &nap::audio::ControlInstance::stop)
RTTI_END_CLASS



namespace nap
{

	namespace audio
	{

		std::unique_ptr<AudioObjectInstance> Control::createInstance(NodeManager& nodeManager, utility::ErrorState& errorState)
		{
			auto instance = std::make_unique<ControlInstance>();

			audio::SafePtr<Translator<float>> translator;
			if (mEqualPowerTable != nullptr)
				translator = mEqualPowerTable->getTable();
			if (!instance->init(nodeManager, translator, errorState))
				return nullptr;

			instance->setValue(mValue);

			return std::move(instance);
		}


		bool ControlInstance::init(NodeManager& nodeManager, audio::SafePtr<Translator<float>>& translator, utility::ErrorState& errorState)
		{
			mControlNode = nodeManager.makeSafe<ControlNode>(nodeManager);

			if (translator != nullptr)
				mControlNode->setTranslator(translator);

			return true;
		}




	}

}