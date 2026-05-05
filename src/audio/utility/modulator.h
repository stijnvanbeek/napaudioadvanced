#pragma once

#include <parameternumeric.h>
#include <fcurve.h>
#include <audio/utility/audiotypes.h>

namespace nap
{

    // Forward declarations
    class Core;

    namespace audio
    {


        class NAPAPI ModulatorSource : public Resource
        {
            RTTI_ENABLE(Resource)

        public:
            ModulatorSource() = default;
            virtual ControllerValue update() = 0;
        };


        class NAPAPI ModulatorTarget : public ModulatorSource
        {
            RTTI_ENABLE(ModulatorSource)

        public:
            enum ModulationType { add, multiply };
            struct Modulation
            {
                ModulationType mType;
                ResourcePtr<ModulatorSource> mSource;
                ResourcePtr<ModulatorSource> mAmount;
                ControllerValue mDefaultAmount = 1.f;
                ControllerValue mOffset = 0.f;
            };

            std::vector<Modulation> mModulations;

            ModulatorTarget() = default;
            ControllerValue modulate(ControllerValue input);
        };


        class NAPAPI ModulatorParameter : public ParameterFloat
        {
            RTTI_ENABLE(ParameterFloat)

        public:
            enum ValueType { normal, frequency, amplitude };

            ValueType mValueType = normal;
            ControllerValue mPower = 1.f;

            ModulatorParameter() = default;
            ControllerValue getValueNormalized();
            void setValueNormalized(ControllerValue value);

            ControllerValue normalize(ControllerValue input);
            ControllerValue denormalize(ControllerValue input);
        };


        class NAPAPI ParameterModulator : public ModulatorTarget
        {
            RTTI_ENABLE(ModulatorTarget)

        public:
            ResourcePtr<ModulatorParameter> mParameter;

            ParameterModulator() = default;

            // Inherited from ModulatorSource
            ControllerValue update() override;

            void setInput(ControllerValue input) { mNormalizedInput = mParameter->normalize(input); }
            void setInputNormalized(ControllerValue input) { mNormalizedInput = input; }
            ControllerValue getInput() const { return mParameter->denormalize(mNormalizedInput); }
            ControllerValue getInputNormalized() const { return mNormalizedInput; }

        private:
            ControllerValue mNormalizedInput = 0.f;
        };


        class NAPAPI CurveModulator : public ModulatorSource
        {
            RTTI_ENABLE(ModulatorSource)

        public:
            ResourcePtr<math::FloatFCurve> mCurve;
            ControllerValue mDefaultSpeed = 1.f;
            ResourcePtr<ModulatorSource> mSpeed;

            CurveModulator(Core& core);

            ControllerValue update() override;

        private:
            double mTime = 0.f;
            Core& mCore;
        };
        
        
    }

}