#include <se/utils/Log.h>
#include <se/graphics/GraphicsEngine.h>
#include "Editor.h"
#include "EntityPanel.h"
#include "ImGuiRenderer.h"

namespace editor {

	Editor::Editor() :
		se::app::Application(
			{ kTitle, kWidth, kHeight },
			{	kMaxManifolds, kMinFDifference, kMaxCollisionIterations,
				kContactPrecision, kContactSeparation, kMaxRayCasterIterations
			},
			kUpdateTime
		),
		mScene(nullptr), mImGuiInput(nullptr), mMenuBar(nullptr), mEntityPanel(nullptr)
	{
		if (mState != AppState::Error) {
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGui::StyleColorsDark();

			ImGuiIO& io = ImGui::GetIO();
			io.DisplaySize = ImVec2(kWidth, kHeight);
			io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
			io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

			auto imGuiRenderer = std::make_unique<ImGuiRenderer>("ImGuiRenderer");
			auto& renderGraph = mExternalTools->graphicsEngine->getRenderGraph();
			auto renderer2D = renderGraph.getNode("renderer2D");
			imGuiRenderer->findInput("target")->connect( renderer2D->findOutput("target") );
			renderGraph.addNode( std::move(imGuiRenderer) );

			mImGuiInput = new ImGuiInput(*mEventManager);
			mMenuBar = new MenuBar(*this);
			mEntityPanel = new EntityPanel(*this);
		}
		else {
			SOMBRA_FATAL_LOG << "Couldn't create the Editor: The Application has errors";
		}
	}


	Editor::~Editor()
	{
		destroyScene();
		if (mEntityPanel) { delete mEntityPanel; }
		if (mMenuBar) { delete mMenuBar; }
		if (mImGuiInput) { delete mImGuiInput; }
		ImGui::DestroyContext();
	}


	void Editor::createScene(const char* name)
	{
		mScene = new se::app::Scene(name, *this);
	}


	void Editor::destroyScene()
	{
		if (mScene) {
			delete mScene;
			mScene = nullptr;
		}
	}

// Private functions
	void Editor::onUpdate(float deltaTime)
	{
		SOMBRA_DEBUG_LOG << "Init (" << deltaTime << ")";

		Application::onUpdate(deltaTime);

		const auto& windowData = mExternalTools->windowManager->getWindowData();
		ImGuiIO& io = ImGui::GetIO();
		io.DeltaTime = deltaTime;
		io.DisplaySize = ImVec2(windowData.width, windowData.height);
	}


	void Editor::onRender()
	{
		ImGui::NewFrame();

		static bool show = true;
		ImGui::ShowDemoWindow(&show);

		mMenuBar->render();
		mEntityPanel->render();

		Application::onRender();
	}

}
