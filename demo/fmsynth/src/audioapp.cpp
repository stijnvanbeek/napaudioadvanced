#include "audioapp.h"

#include <parameteroptionlist.h>

// Nap includes
#include <nap/core.h>
#include <renderablemeshcomponent.h>
#include <orthocameracomponent.h>
#include <mathutils.h>
#include <scene.h>
#include <inputrouter.h>
#include <imgui/imgui.h>
#include <renderwindow.h>
#include <renderservice.h>
#include <nap/logger.h>
#include <parametersimple.h>

// Register this application with RTTI, this is required by the AppRunner to 
// validate that this object is indeed an application
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::AudioTestApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{

    /**
     * Register an editor for the the ParameterOptionList parameter type with the ParameterGUIService
     */
    void registerParameterEditors(ParameterGUIService& parameterGUIService)
    {
        parameterGUIService.registerParameterEditor(RTTI_OF(ParameterOptionList), [](Parameter& parameter)
        {
            ParameterOptionList* optionList = rtti_cast<ParameterOptionList>(&parameter);

            std::vector<rttr::string_view> items(optionList->getOptions().begin(), optionList->getOptions().end());

            int value = optionList->getValue();

            ImGui::PushID(&parameter);

            if (ImGui::Combo(optionList->getDisplayName().c_str(), &value,
                             [](void* data, int index, const char** out_text)
                             {
                                 std::vector<rttr::string_view>* items = (std::vector<rttr::string_view>*)data;
                                 *out_text = (*items)[index].data();
                                 return true;
                             },
                             &items, items.size()))
            {
                optionList->setValue(value);
            }

            ImGui::PopID();

        });
    }


	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool AudioTestApp::init(utility::ErrorState& error)
	{
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();
        mMidiService	= getCore().getService<nap::MidiService>();

        // Register the custom parameter editor
        registerParameterEditors(*getCore().getService<ParameterGUIService>());

		// Get resource manager and load and deserialize app structure
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile(appJson, error))
			return false;

        // Find the parameter group
		mParameterGroup = mResourceManager->findObject<ParameterGroup>("Parameters");
		if (mParameterGroup == nullptr)
        {
            error.fail("Couldn't find parameter group");
            return false;
        }

        // Find the window resource
        mRenderWindow = mResourceManager->findObject<RenderWindow>("Window0");
        if (mRenderWindow == nullptr)
        {
            error.fail("Couldn't find render window");
            return false;
        }

        // Find the parameter GUI
        mParameterGUI = mResourceManager->findObject<ParameterGUI>("ParameterGUI");
        if (mParameterGUI == nullptr)
        {
            error.fail("Couldn't findparameter GUI");
            return false;
        }

		return true;
	}
	
	
	void AudioTestApp::update(double deltaTime)
	{
        ImGui::Begin(appName.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		if (mParameterGroup != nullptr)
		{
			if (mParameterGUI != nullptr)
				mParameterGUI->show(false);
		}
		ImGui::NewLine();
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
        ImGui::End();
    }

	
	void AudioTestApp::render()
	{
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Begin recording the render commands for the main render window
		if (mRenderService->beginRecording(*mRenderWindow))
		{
			// Begin render pass
			mRenderWindow->beginRendering();

			// Render GUI elements
			mGuiService->draw();

			// Stop render pass
			mRenderWindow->endRendering();

			// End recording
			mRenderService->endRecording();
		}

		// Proceed to next frame
		mRenderService->endFrame();	}
	
	
	/**
	 * Occurs when the event handler receives a window message.
	 * You generally give it to the render service which in turn forwards it to the right internal window. 
	 * On the next update the render service automatically processes all window events. 
	 * If you want to listen to specific events associated with a window it's best to listen to a window's mWindowEvent signal
	 */
	void AudioTestApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	 * Called by the app loop. It's best to forward messages to the input service for further processing later on
	 * In this case we also check if we need to exit the running app or forward certain key input events as midi notes
	 */
	void AudioTestApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		// Escape the loop when esc is pressed
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit();
		}

        // In case of a key event try converting it to a midi note and forward to the midi service
		auto keyEvent = rtti_cast<KeyEvent>(inputEvent.get());
		if (keyEvent != nullptr)
		{
			auto midiEvent = mKeyToMidiConverter.processKeyEvent(*keyEvent);
			if (midiEvent != nullptr)
				mMidiService->enqueueEvent(std::move(midiEvent));
		}

		mInputService->addEvent(std::move(inputEvent));
	}

	
	int AudioTestApp::shutdown()
	{
		mRenderWindow = nullptr;
		return 0;
	}
}
