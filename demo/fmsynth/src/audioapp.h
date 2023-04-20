 #pragma once

// Local includes
#include "keytomidi.h"

// Mod nap render includes
#include <renderwindow.h>

// Nap includes
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <imguiservice.h>
#include <midiservice.h>
#include <app.h>
#include <parameter.h>
#include <parametergui.h>
#include <midiinputport.h>

/**
 * Name of the app structure file of the audio application
 */
const std::string appJson = "fmsynth.json";

/**
 * Name of the application as it is displayed in the window caption.
 */
const std::string appName = "FM Synth";


namespace nap
{

	using namespace rtti;


	void registerParameterEditors(ParameterGUI& parameterGUI);

	/**
	 * This App functions as a template for an application that generates audio based on midi input and/or a selection of parameters.
	 */
	class AudioTestApp : public App
	{
		RTTI_ENABLE(App)
	public:
		AudioTestApp(nap::Core& core) : App(core)	{ }
		~AudioTestApp() { mRenderWindow = nullptr; }

		/**
		 *	Initialize app specific data structures
		 */
		bool init(utility::ErrorState& error) override;
		
		/**
		 *	Update is called before render, performs all the app logic
		 */
		void update(double deltaTime) override;

		/**
		 *	Render is called after update, pushes all renderable objects to the GPU
		 */
		void render() override;

		/**
		 *	Forwards the received window event to the render service
		 */
		void windowMessageReceived(WindowEventPtr windowEvent) override;
		
		/**
		 *  Forwards the received input event to the input service
		 */
		void inputMessageReceived(InputEventPtr inputEvent) override;
		
		/**
		 *	Called when loop finishes
		 */
		int shutdown() override;

	private:
		// Nap Services
		RenderService* mRenderService = nullptr;						// Render Service that handles render calls
		ResourceManager* mResourceManager = nullptr;					// Manages all the loaded resources
		SceneService* mSceneService = nullptr;							// Manages all the objects in the scene
		InputService* mInputService = nullptr;							// Input service for processing input
		IMGuiService* mGuiService = nullptr;							// Manages gui related update / draw calls
		MidiService* mMidiService = nullptr;                            // Service managing midi input events and distributing across MidiInputComponents

        RGBAColor8 mTextHighlightColor = { 0xC8, 0x69, 0x69, 0xFF };    // GUI text highlight color
        ObjectPtr<RenderWindow> mRenderWindow = nullptr;			    // Pointer to the render window
        KeyToMidiConverter mKeyToMidiConverter;                         // Helper object that converts character key input to midi note events
		ObjectPtr<ParameterGroup> mParameterGroup = nullptr;            // The parameter group containing the parameters for the audio generation
		ObjectPtr<ParameterGUI> mParameterGUI = nullptr;                // Gui object to display and edit the parameter group in the GUI
	};
}
