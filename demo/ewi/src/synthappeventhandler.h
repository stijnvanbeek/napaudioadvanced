/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <appeventhandler.h>
#include <sdleventconverter.h>

namespace nap
{
    // Forward Declares
    class IMGuiService;

    /**
     * This is basically a copy of GUIAppEventHandler with the only distinction that keyboard events are always forwarded to the app.
     * This enables the user to play the virtual keyboard while the mouse is being used for changing parameters using the GUI
     */
    class SynthAppEventHandler : public AppEventHandler
    {
        RTTI_ENABLE(AppEventHandler)
    public:
        SynthAppEventHandler(App& app);

        /**
         * This call creates the SDL Input Converter.
         */
        virtual void start() override;

        /**
         * This call polls the various SDL messages and filters them based on GUI activity.
         * If a GUI element is actively used the events are still forwarded to the running app in order to be able to play the virtual midi keyboard
         */
        virtual void process() override;

        /**
         * This call deletes the input converter
         */
        virtual void shutdown() override;

        /**
         * Tells the input system if touch input also generates mouse events, next to touch events.
         * On most systems touch input is coupled to mouse input. This is controlled on an operating system level.
         * This call tries to explicitly tell the input system to couple or decouple both events.
         * When decoupled, touch events (instead of mouse events) are forwarded to the GUI.
         * @param value if touch input generates mouse events.
         * @return if the system accepted the hint.
         */
        bool setTouchGeneratesMouseEvents(bool value);

    private:
        std::unique_ptr<SDLEventConverter> mEventConverter = nullptr;
        IMGuiService* mGuiService = nullptr;
        bool mTouchGeneratesMouseEvents = true;
    };
}
