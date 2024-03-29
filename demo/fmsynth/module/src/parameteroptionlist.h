/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <parameter.h>
#include <nap/signalslot.h>

namespace nap
{

    /**
     * Works very much like an enum parameter except that the available values and names can be determined at runtime.
     */
    class NAPAPI ParameterOptionList : public Parameter
    {
        RTTI_ENABLE(Parameter)
        
    public:
        ParameterOptionList() = default;

        int mValue = 0; ///< Property: 'Value'
		std::vector<std::string> mOptions; ///< Property: 'Options'

        /**
         * Sets the list of available options. Resets the current index to 0.
         */
        void setOptions(const std::vector<std::string>& options);

        // Inherited from Parameter
        void setValue(const Parameter& value) override;

        /**
         * Sets the current option choice as index in the list
         */
        void setValue(int index);

        /**
         * Sets the current option using its name. Returns false if the option is not found.
         */
        bool setOption(const std::string& optionName);

        /**
         * @return the index of the current choice.
         */
        int getValue() const { return mValue; }

        /**
         * @return the name of the current choice
         */
        const std::string& getOptionName() const { return mOptions[mValue]; }

        /**
         * @return list of all available options.
         */
        const std::vector<std::string>& getOptions() const { return mOptions; }

        /**
         * Signal triggered whenever the choice has changed.
         */
        Signal<int> valueChanged;
    };

}
