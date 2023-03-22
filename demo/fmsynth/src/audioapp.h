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

// Audio includes
#include <audio/component/playbackcomponent.h>

/*
 * Select your serialized json audio demo app here:
 */
const std::string appJson = "fmsynth.json";

namespace nap
{

	using namespace rtti;


	void registerParameterEditors(ParameterGUI& parameterGUI);

	/**
     * This demo application shows how to playback an audio file using audio::PlaybackComponent.
     * It shows how to start and stop playback and how to modify playback parameters.
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
		RenderService* mRenderService = nullptr;						//< Render Service that handles render calls
		ResourceManager* mResourceManager = nullptr;					//< Manages all the loaded resources
		SceneService* mSceneService = nullptr;							//< Manages all the objects in the scene
		InputService* mInputService = nullptr;							//< Input service for processing input
		IMGuiService* mGuiService = nullptr;							//< Manages gui related update / draw calls
		MidiService* mMidiService = nullptr;
        RGBAColor8 mTextHighlightColor = { 0xC8, 0x69, 0x69, 0xFF };    //< GUI text highlight color
		KeyToMidiConverter mKeyToMidiConverter;
		ObjectPtr<ParameterGroup> mParameterGroup = nullptr;
		ObjectPtr<RenderWindow> mRenderWindow = nullptr;			//< Pointer to the render window
		ObjectPtr<ParameterGUI> mParameterGUI = nullptr;
		ObjectPtr<MidiInputPort> mMidiInputPort = nullptr;
	};
}
