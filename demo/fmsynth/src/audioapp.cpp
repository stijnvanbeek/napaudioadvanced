/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "audioapp.h"

// Nap includes
#include <nap/core.h>
#include <renderablemeshcomponent.h>
#include <perspcameracomponent.h>
#include <mathutils.h>
#include <scene.h>
#include <imgui/imgui.h>
#include <renderwindow.h>
#include <renderservice.h>
#include <nap/logger.h>
#include <parametersimple.h>
#include <renderglobals.h>

#include "fftaudionodecomponent.h"
#include "meshutils.h"

// Register this application with RTTI, this is required by the AppRunner to 
// validate that this object is indeed an application
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::AudioTestApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{


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

        // Create the window resource
    	mRenderWindow = std::make_unique<RenderWindow>(getCore());
    	mRenderWindow->mWidth = 600;
    	mRenderWindow->mHeight = 500;
    	if (!mRenderWindow->init(error))
    		return false;

    	// Create the parameter GUI
    	mParameterGUI = std::make_unique<ParameterGUI>(getCore());
    	mParameterGUI->mParameterGroup = mParameterGroup;
    	mParameterGUI->mSerializable = false;
    	if (!mParameterGUI->init(error))
    		return false;

    	// Create the midi input port
    	mMidiInputPort = std::make_unique<MidiInputPort>(getCore());
    	if (!mMidiInputPort->start(error))
    		return false;

    	auto scene = mResourceManager->findObject<Scene>("Scene");
    	mPlaneEntity = scene->findEntity("PlaneEntity");
    	mCameraEntity = scene->findEntity("CameraEntity");
    	mSynthEntity = scene->findEntity("SynthEntity");

    	auto& mesh = mResourceManager->findObject<Mesh>("PlaneMesh")->getMeshInstance();
    	Vec3VertexAttribute& positions = mesh.getOrCreateAttribute<glm::vec3>(vertexid::position);
    	for (int i = 0; i < positions.getCount(); i++)
    		originalHeights.push_back(positions[i].z);

		return true;
	}
	
	
	void AudioTestApp::update(double deltaTime)
	{
    	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    	ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.0f, 0.0f, 0.0f, 0.7f));
    	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

    	ImGui::SetNextWindowPos(ImVec2(150, 25));
    	ImGui::SetNextWindowSize(ImVec2(400, 500));
    	ImGui::SetNextWindowBgAlpha(0.0f);

    	ImGui::Begin("FM Synth", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
    	if (mParameterGUI != nullptr)
    		mParameterGUI->show(false);
    	ImGui::NewLine();

    	std::string formattedText = nap::utility::stringFormat("Framerate: %.02f", getCore().getFramerate());
    	ImGui::Text(formattedText.c_str());
    	ImGui::End();

    	ImGui::PopStyleVar();
    	ImGui::PopStyleColor();
    	ImGui::PopStyleColor();
    	ImGui::PopStyleColor();
    	ImGui::PopStyleColor();
    	ImGui::PopStyleColor();

    	auto fftComponent = mSynthEntity->findComponent<FFTAudioNodeComponentInstance>();
    	auto& buffer = fftComponent->getFFTBuffer();
    	auto& mesh = mResourceManager->findObject<Mesh>("PlaneMesh")->getMeshInstance();
    	Vec3VertexAttribute& positions = mesh.getOrCreateAttribute<glm::vec3>(vertexid::position);
    	auto& amplitudes = buffer.getAmplitudeSpectrum();
    	for (int i = 0; i < positions.getCount(); ++i)
    	{
    		int index = static_cast<int>(pow(positions[i].x + 0.5f, 2.5) * (amplitudes.size() - 1));
    		index = math::clamp<int>(index, 0, amplitudes.size() - 1);

    		float displacement = amplitudes[index] * 10.f;
			positions[i].z = originalHeights[i] + displacement;
    	}
    	utility::ErrorState error_state;
    	bool result = mesh.update(error_state);
    	assert(result);
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

			std::vector<nap::RenderableComponentInstance*> components_to_render;
			auto& plane = mPlaneEntity->getComponent<nap::RenderableMeshComponentInstance>();
			components_to_render.emplace_back(&plane);

			// Find the perspective camera
			auto& camera = mCameraEntity->getComponent<nap::PerspCameraComponentInstance>();

			// Render the world with the right camera directly to screen
			mRenderService->renderObjects(*mRenderWindow, camera, components_to_render);

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
