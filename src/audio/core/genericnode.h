/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <audio/core/audionode.h>


namespace nap
{
    
    namespace audio
    {

        /**
         * This class is used as a generic interface for Node wrappers for DSP building blocks from other DSP platforms.
         * It exposes a number of audio inputs and outputs and a series of named float parameters.
         * For example: Faust or Max/MSP objects.
         */
        class NAPAPI GenericNode : public Node
        {
            RTTI_ENABLE(nap::audio::Node)
        public:
            GenericNode(NodeManager& manager) : Node(manager) { }

            /**
             * @return number of audio input channels of this node.
             */
            int getInputCount() const { return mInputs.size(); }

            /**
             * @return number of audio output channels of this node.
             */
            int getOutputCount() const { return mOutputs.size(); }

            /**
             * Checks if a parameter with a given name exists.
             * @param name The name of the requested parameter.
             * @return true if the parameter exists.
             */
            bool hasParameter(const std::string& name) const { return mParameters.find(name) != mParameters.end(); }

            /**
             * @return A vector containing the names of all parameters of the node.
             */
            std::vector<std::string> getParameterNames() const;

            /**
             * @param index The index of the requested input pin
             * @return Pointer to the requested input pin.
             */
            InputPin* getInput(unsigned int index) { return mInputs[index].get(); }

            /**
             * @param index The index of the requested output pin.
             * @return Pointer to the requested output pin.
             */
            OutputPin* getOutput(unsigned int index) { return mOutputs[index].get(); }

            /**
             * Request value of a named parameter.
             * @param name Name of the parameter from which the value is requested.
             * @return Pointer to the parameter value if the parameter exists, otherwise nullptr.
             */
            float* getParameterValue(const std::string& name) const;

            /**
             * Searches for a parameter with a given name and sets its value if found.
             * @param name The name of the requested parameter.
             * @param value The new value for the parameter.
             * @return True if the parameter is found and the value set.
             */
            bool setParameterValue(const std::string& name, float value);

            /**
             * Searches for a parameter and translates a value between 0 and 1 to the according range.
             * @param name The name of the requested parameter
             * @param value Value between 0 and 1 that will be mapped onto the range of the parameter and assigned as new value.
             * @return True if the parameter was found and its value set.
             */
            bool setParameterNormalizedValue(const std::string& name, float value);

            /**
             * Add a new parameter to the interface.
             * @param name The name of the new parameter.
             * @param zone Pointer to a location in memory that (already) holds the value of this parameter.
             * Make sure that this memory is retained during the lifetime of this node.
             * This slightly archaic and unsafe way of accessing memory is here for compatibility with Faust and Max/MSP.
             * @param min Minimum value of the parameter used for mapping new values.
             * @param max Maximum value of the parameter used for mapping new values.
             */
            void addParameter(const std::string& name, float* zone, float min, float max);
            
        protected:
            void setInputCount(unsigned int count);
            void setOutputCount(unsigned int count);
            
        private:
            class Parameter {
            public:
                float* mZone = nullptr;
                float mMin = 0.f;
                float mMax = 1.f;
            };
            
            std::vector<std::unique_ptr<InputPin>> mInputs;
            std::vector<std::unique_ptr<OutputPin>> mOutputs;
            std::map<std::string, Parameter> mParameters;
        };
        
    }
    
}
