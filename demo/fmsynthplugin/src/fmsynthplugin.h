#pragma once

#include <audioplugin.h>
#include <parametergui.h>
#include <renderservice.h>
#include <imguiservice.h>
#include <nap/core.h>
#include <entity.h>

#include "smoothdamp.h"

namespace nap
{

	class FMSynthPlugin : public AudioPlugin
	{
		RTTI_ENABLE(AudioPlugin)

	public:
		FMSynthPlugin(Core& core) : AudioPlugin(core) { }

		bool init(utility::ErrorState& errorState) override;
		void update(double deltaTime) override;
		void render(nap::RenderWindow* renderWindow) override;
		void shutdown() override;
		glm::vec2 getRenderWindowSize() override { return glm::vec2(600.f, 500.f); }

	private:
		std::unique_ptr<nap::ParameterGUI> mParameterGUI = nullptr;
		RenderService* mRenderService = nullptr;
		IMGuiService* mGuiService = nullptr;

		rtti::ObjectPtr<EntityInstance> mPlaneEntity = nullptr;
		rtti::ObjectPtr<EntityInstance> mCameraEntity = nullptr;
		rtti::ObjectPtr<EntityInstance> mSynthEntity = nullptr;
		rtti::ObjectPtr<Mesh> mMesh = nullptr;
		std::vector<float> mOriginalHeights;
		std::vector<math::SmoothOperator<float>> mFFTBufferSmoothing;
	};

}
