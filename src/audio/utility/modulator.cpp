#include "modulator.h"

#include <audio/utility/audiofunctions.h>
#include <nap/core.h>

RTTI_DEFINE_BASE(nap::audio::ModulatorSource)

RTTI_BEGIN_ENUM(nap::audio::ModulatorTarget::ModulationType)
    RTTI_ENUM_VALUE(nap::audio::ModulatorTarget::ModulationType::add, "add"),
    RTTI_ENUM_VALUE(nap::audio::ModulatorTarget::ModulationType::multiply, "multiply")
RTTI_END_ENUM

RTTI_BEGIN_STRUCT(nap::audio::ModulatorTarget::Modulation)
    RTTI_PROPERTY("ModulationType", &nap::audio::ModulatorTarget::Modulation::mType, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Source", &nap::audio::ModulatorTarget::Modulation::mSource, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Amount", &nap::audio::ModulatorTarget::Modulation::mAmount, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("DefaultAmount", &nap::audio::ModulatorTarget::Modulation::mDefaultAmount, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Offset", &nap::audio::ModulatorTarget::Modulation::mOffset, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::ModulatorTarget)
    RTTI_PROPERTY("Modulations", &nap::audio::ModulatorTarget::mModulations, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_ENUM(nap::audio::ModulatorParameter::ValueType)
    RTTI_ENUM_VALUE(nap::audio::ModulatorParameter::ValueType::normal, "normal"),
    RTTI_ENUM_VALUE(nap::audio::ModulatorParameter::ValueType::frequency, "frequency"),
    RTTI_ENUM_VALUE(nap::audio::ModulatorParameter::ValueType::amplitude, "amplitude")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::audio::ModulatorParameter)
    RTTI_PROPERTY("ValueType", &nap::audio::ModulatorParameter::mValueType, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Power", &nap::audio::ModulatorParameter::mPower, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::audio::ParameterModulator)
    RTTI_PROPERTY("Parameter", &nap::audio::ParameterModulator::mParameter, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::CurveModulator)
    RTTI_CONSTRUCTOR(nap::Core&)
    RTTI_PROPERTY("Curve", &nap::audio::CurveModulator::mCurve, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{

    namespace audio
    {

        ControllerValue ModulatorTarget::modulate(ControllerValue input)
        {
            ControllerValue result = input;

            // First all additions
            for (Modulation& mod : mModulations)
                if (mod.mType == add)
                {
                    auto amount = mod.mDefaultAmount;
                    if (mod.mAmount != nullptr)
                        amount = mod.mAmount->update();
                    result = result + mod.mOffset + mod.mSource->update() * amount;
                }

            // Then all multiplications
            for (Modulation& mod : mModulations)
                if (mod.mType == multiply)
                {
                    auto amount = mod.mDefaultAmount;
                    if (mod.mAmount != nullptr)
                        amount = mod.mAmount->update();
                    result = result * mod.mSource->update() * amount;
                }

            math::clamp(result, 0.0f, 1.0f);
            return result;
        }

        
        ControllerValue ModulatorParameter::getValueNormalized()
        {
            return normalize(mValue);
        }

        
        void ModulatorParameter::setValueNormalized(ControllerValue value)
        {
            mValue = denormalize(value);
        }


        ControllerValue ModulatorParameter::normalize(ControllerValue input)
        {
            if (mValueType == amplitude)
            {
                ControllerValue result = toDB(input);
                if (result < 48.f)
                    result = 48.f;
                result = (result + 48.f) / 48.f;
                return result;
            }
            else if (mValueType == frequency)
            {
                ControllerValue result = ftom(input);
                result = result / 127.f;
                return result;
            }
            else
            {
                ControllerValue result = (input - mMinimum) / (mMaximum - mMinimum);
                result = pow(result, 1 / mPower);
                return result;
            }
        }


        ControllerValue ModulatorParameter::denormalize(ControllerValue input)
        {
            if (mValueType == amplitude)
            {
                ControllerValue result = dbToA(input * 48.f - 48.f);
                return result;
            }
            else if (mValueType == frequency)
            {
                ControllerValue result = mtof(input * 127.f);
                return result;
            }
            else
            {
                ControllerValue result = pow(input, mPower);
                return mMinimum + (mMaximum - mMinimum) * result;
            }
        }


        ControllerValue ParameterModulator::update()
        {
            ControllerValue result = modulate(mNormalizedInput);
            mParameter->setValueNormalized(result);
            return result;
        }


        CurveModulator::CurveModulator(Core &core) : mCore(core)
        {
        }


        ControllerValue CurveModulator::update()
        {
            mTime = mCore.getElapsedTime();
            while (mTime > 1.f)
                mTime -= 1.f;
            return mCurve->evaluate(mTime);
        }

    }

}