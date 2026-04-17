#include "fmsynthplugin.h"

#include <perspcameracomponent.h>
#include <renderablemeshcomponent.h>
#include <nap/resourcemanager.h>
#include <renderglobals.h>
#include <scene.h>
#include <fftaudionodecomponent.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FMSynthPlugin)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{

	bool FMSynthPlugin::init(utility::ErrorState &errorState)
	{
		mRenderService = getCore().getService<RenderService>();
		mGuiService = getCore().getService<IMGuiService>();

		auto resourceManager = getCore().getResourceManager();
		auto parameterGroup = resourceManager->findObject<nap::ParameterGroup>("Parameters").get();
		if (parameterGroup == nullptr)
		{
			errorState.fail("Can't find parameter group: Parameters");
			return false;
		}

		for (auto& parameter : parameterGroup->mMembers)
			registerParameterSignal(*parameter);

		mParameterGUI = std::make_unique<nap::ParameterGUI>(getCore());
		mParameterGUI->mParameterGroup = parameterGroup;
		mParameterGUI->mSerializable = false;
		if (!mParameterGUI->init(errorState))
		{
			errorState.fail("Failed to initialize parameter GUI.");
			return false;
		}

		auto scene = resourceManager->findObject<Scene>("Scene");
		mPlaneEntity = scene->findEntity("PlaneEntity");
		mCameraEntity = scene->findEntity("CameraEntity");
		mSynthEntity = scene->findEntity("SynthEntity");
		mMesh = resourceManager->findObject<Mesh>("PlaneMesh");

		auto& mesh = resourceManager->findObject<Mesh>("PlaneMesh")->getMeshInstance();
		Vec3VertexAttribute& positions = mesh.getOrCreateAttribute<glm::vec3>(vertexid::position);
		for (int i = 0; i < positions.getCount(); i++)
			mOriginalHeights.push_back(positions[i].z);

		return true;
	}


	void FMSynthPlugin::update(double deltaTime)
	{
		// Perform some minimalistic gui styling
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

		ImGui::SetNextWindowPos(ImVec2(150, 25));
		ImGui::SetNextWindowSize(ImVec2(400, 500));
		ImGui::SetNextWindowBgAlpha(0.0f);

		// Draw the parameter GUI in an invisible window
		ImGui::Begin("FM Synth", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
		if (mParameterGUI != nullptr)
			mParameterGUI->show(false);
		ImGui::NewLine();

		// Draw the framerate
		std::string formattedText = nap::utility::stringFormat("Framerate: %.02f", getCore().getFramerate());
		ImGui::Text(formattedText.c_str());
		ImGui::End();

		// Pop the style from the stack
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		// Grab the amplitudes from the FFT buffer
		auto fftComponent = mSynthEntity->findComponent<FFTAudioNodeComponentInstance>();
		auto& buffer = fftComponent->getFFTBuffer();
		auto& amplitudes = buffer.getAmplitudeSpectrum();
		if (mFFTBufferSmoothing.size() != amplitudes.size())
			for (int i = 0; i < amplitudes.size(); i++)
				mFFTBufferSmoothing.push_back(nap::math::SmoothOperator<float>(amplitudes[i], 0.1f));
		else
		{
			for (int i = 0; i < mFFTBufferSmoothing.size(); i++)
				mFFTBufferSmoothing[i].update(amplitudes[i], deltaTime);
		}

		// Get the positions vertex buffer from the mesh
		Vec3VertexAttribute& positions = mMesh->getMeshInstance().getOrCreateAttribute<glm::vec3>(vertexid::position);
		for (int i = 0; i < positions.getCount(); ++i)
		{
			// Calculate the index in the amplitudes array from the x position of the vertex
			int index = static_cast<int>(pow(positions[i].x + 0.5f, 2.5) * (amplitudes.size() - 1));
			index = math::clamp<int>(index, 0, amplitudes.size() - 1);

			// Displace the z coordinate of the vertex
			float displacement = mFFTBufferSmoothing[index].getValue() * 10.f;
			if (std::isnan(displacement))
				mFFTBufferSmoothing[index].setValue(0.f);
			positions[i].z = mOriginalHeights[i] + displacement;
		}
		// Push the mesh with the new vertex positions to the GPU
		utility::ErrorState error_state;
		bool result = mMesh->getMeshInstance().update(error_state);
		assert(result);
	}


	void FMSynthPlugin::render(nap::RenderWindow *renderWindow)
	{
		mRenderService->beginFrame();
		if (renderWindow != nullptr)
		{
			if (mRenderService->beginRecording(*renderWindow))
			{
				renderWindow->beginRendering();

				// Find the renderable plane
				auto& plane = mPlaneEntity->getComponent<nap::RenderableMeshComponentInstance>();

				// Find the camera
				auto& camera = mCameraEntity->getComponent<nap::PerspCameraComponentInstance>();

				// Render the plane with the camera
				mRenderService->renderObjects(*renderWindow, camera, { &plane });

				// Draw the GUI on top
				mGuiService->draw();

				renderWindow->endRendering();
				mRenderService->endRecording();
			}
		}
		mRenderService->endFrame();
	}


	void FMSynthPlugin::shutdown()
	{
		mParameterGUI->onDestroy();
		mParameterGUI = nullptr;
	}

}
