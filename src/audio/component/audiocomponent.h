/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Nap includes
#include <component.h>
#include <componentptr.h>
#include <rtti/objectptr.h>
#include <nap/resourceptr.h>

// Audio includes
#include <audio/component/audiocomponentbase.h>
#include <audio/core/audionode.h>
#include <audio/core/audioobject.h>

namespace nap
{
    
    namespace audio
    {
    
        class AudioComponentInstance;
        
        
        /**
         * Component that wraps an audio object that generates audio output for one or more channels.
         */
        class NAPAPI AudioComponent : public AudioComponentBase
        {
            RTTI_ENABLE(nap::audio::AudioComponentBase)
            DECLARE_COMPONENT(AudioComponent, AudioComponentInstance)
            
        public:
            AudioComponent() : AudioComponentBase() { }
            
            ResourcePtr<AudioObject> mObject;                 ///< Property: 'Object' The audio object that is wrapped by this component
            
            std::vector<ComponentPtr<AudioComponent>> mLinks; ///< Property: 'Links' Pointers to audio components whose audio objects can be linked to from within this component
        };

        
        /**
         * Instance of a component that wraps an audio object that generates audio output for one or more channels
         */
        class NAPAPI AudioComponentInstance : public AudioComponentBaseInstance
        {
            RTTI_ENABLE(nap::audio::AudioComponentBaseInstance)
            
        public:
            AudioComponentInstance(EntityInstance& entity, Component& resource) : AudioComponentBaseInstance(entity, resource) { }

            // Inherited from AudioComponentBaseInstance
            bool init(utility::ErrorState& errorState) override;
            int getChannelCount() const override { return mObject->getChannelCount(); }
            virtual OutputPin* getOutputForChannel(int channel) override { return mObject->getOutputForChannel(channel); }

            /**
             * @return the wrapped audio object. Nullptr if T does not match the object type.
             */
            template <typename T>
            T* getObject() { return rtti_cast<T>(mObject.get()); }

            /**
             * @return the wrapped audio object
             */
            AudioObjectInstance* getObjectNonTyped();
            
        private:
            std::unique_ptr<AudioObjectInstance> mObject = nullptr;
        };

    }
        
}
