#include <imgui.h>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include <se/utils/Log.h>
#include <se/graphics/Renderer.h>
#include <se/graphics/GraphicsEngine.h>
#include <se/graphics/core/UniformVariable.h>
#include <se/graphics/core/GraphicsOperations.h>
#include <se/app/io/MeshLoader.h>
#include <se/app/io/ShaderLoader.h>
#include <se/app/graphics/RenderableShader.h>
#include <se/app/TransformsComponent.h>
#include <se/app/CameraComponent.h>
#include <se/app/MeshComponent.h>
#include <se/app/events/ContainerEvent.h>
#include "Editor.h"
#include "ViewportControl.h"
#include "EntityPanel.h"
#include "ComponentPanel.h"
#include "RepositoryPanel.h"
#include "SceneNodesPanel.h"
#include "Gizmo.h"

namespace editor {

	Editor::Editor() :
		se::app::Application(
			{ kTitle, kWidth, kHeight },
			{	kMaxManifolds, kMinFDifference, kMaxCollisionIterations,
				kContactPrecision, kContactSeparation, kMaxRayCasterIterations
			},
			kUpdateTime
		),
		mImGuiContext(nullptr), mImGuiInput(nullptr), mImGuiRenderer(nullptr),
		mMenuBar(nullptr), mPanelIdCount(0),
		mViewportEntity(se::app::kNullEntity), mGridEntity(se::app::kNullEntity),
		mActiveEntity(se::app::kNullEntity),
		mScene(nullptr)
	{
		SOMBRA_INFO_LOG << "Creating the editor";

		if (mState == AppState::Error) {
			SOMBRA_FATAL_LOG << "Couldn't create the Editor: The Application has errors";
			return;
		}

		mEventManager->subscribe(this, se::app::Topic::Close);

		/**** Create the ImGui context and renderer ****/
		IMGUI_CHECKVERSION();
		mImGuiContext = ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(kWidth, kHeight);
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImGuiStyle* style = &ImGui::GetStyle();
		style->ChildRounding = 4.0f;
		style->FrameBorderSize = 1.0f;
		style->FrameRounding = 2.0f;
		style->GrabMinSize = 7.0f;
		style->PopupRounding = 2.0f;
		style->ScrollbarRounding = 12.0f;
		style->ScrollbarSize = 13.0f;
		style->TabBorderSize = 1.0f;
		style->TabRounding = 2.0f;
		style->WindowRounding = 4.0f;
		style->Colors[ImGuiCol_WindowBg]				= {0.180f, 0.180f, 0.180f, 0.75f};
		style->Colors[ImGuiCol_ChildBg]					= {0.280f, 0.280f, 0.280f, 0.0f};
		style->Colors[ImGuiCol_PopupBg]					= {0.313f, 0.313f, 0.313f, 1.0f};
		style->Colors[ImGuiCol_Border]					= {0.266f, 0.266f, 0.266f, 1.0f};
		style->Colors[ImGuiCol_BorderShadow]			= {0.0f, 0.0f, 0.0f, 0.0f};
		style->Colors[ImGuiCol_FrameBg]					= {0.160f, 0.160f, 0.160f, 1.0f};
		style->Colors[ImGuiCol_FrameBgHovered]			= {0.2f, 0.2f, 0.2f, 1.0f};
		style->Colors[ImGuiCol_FrameBgActive]			= {0.280f, 0.280f, 0.280f, 1.0f};
		style->Colors[ImGuiCol_TitleBg]					= {0.148f, 0.148f, 0.148f, 1.0f};
		style->Colors[ImGuiCol_TitleBgActive]			= {0.148f, 0.148f, 0.148f, 1.0f};
		style->Colors[ImGuiCol_TitleBgCollapsed]		= {0.148f, 0.148f, 0.148f, 1.0f};
		style->Colors[ImGuiCol_MenuBarBg]				= {0.195f, 0.195f, 0.195f, 1.0f};
		style->Colors[ImGuiCol_ScrollbarBg]				= {0.160f, 0.160f, 0.160f, 1.0f};
		style->Colors[ImGuiCol_ScrollbarGrab]			= {0.277f, 0.277f, 0.277f, 1.0f};
		style->Colors[ImGuiCol_ScrollbarGrabHovered]	= {0.3f, 0.3f, 0.3f, 1.0f};
		style->Colors[ImGuiCol_ScrollbarGrabActive]		= {1.0f, 0.391f, 0.0f, 1.0f};
		style->Colors[ImGuiCol_CheckMark]				= {1.0f, 1.0f, 1.0f, 1.0f};
		style->Colors[ImGuiCol_SliderGrab]				= {0.391f, 0.391f, 0.391f, 1.0f};
		style->Colors[ImGuiCol_SliderGrabActive]		= {1.0f, 0.391f, 0.0f, 1.0f};
		style->Colors[ImGuiCol_Button]					= {1.0f, 1.0f, 1.0f, 0.0f};
		style->Colors[ImGuiCol_ButtonHovered]			= {1.0f, 1.0f, 1.0f, 0.156f};
		style->Colors[ImGuiCol_ButtonActive]			= {1.0f, 1.0f, 1.0f, 0.391f};
		style->Colors[ImGuiCol_Header]					= {0.313f, 0.313f, 0.313f, 1.0f};
		style->Colors[ImGuiCol_HeaderHovered]			= {0.469f, 0.469f, 0.469f, 1.0f};
		style->Colors[ImGuiCol_HeaderActive]			= {0.469f, 0.469f, 0.469f, 1.0f};
		style->Colors[ImGuiCol_Separator]				= style->Colors[ImGuiCol_Border];
		style->Colors[ImGuiCol_SeparatorHovered]		= {0.391f, 0.391f, 0.391f, 1.0f};
		style->Colors[ImGuiCol_SeparatorActive]			= {1.0f, 0.391f, 0.0f, 1.0f};
		style->Colors[ImGuiCol_ResizeGrip]				= {1.0f, 1.0f, 1.0f, 0.250f};
		style->Colors[ImGuiCol_ResizeGripHovered]		= {1.0f, 1.0f, 1.0f, 0.670f};
		style->Colors[ImGuiCol_ResizeGripActive]		= {1.0f, 0.391f, 0.0f, 1.0f};
		style->Colors[ImGuiCol_Tab]						= {0.098f, 0.098f, 0.098f, 1.0f};
		style->Colors[ImGuiCol_TabHovered]				= {0.352f, 0.352f, 0.352f, 1.0f};
		style->Colors[ImGuiCol_TabActive]				= {0.195f, 0.195f, 0.195f, 1.0f};
		style->Colors[ImGuiCol_TabUnfocused]			= {0.098f, 0.098f, 0.098f, 1.0f};
		style->Colors[ImGuiCol_TabUnfocusedActive]		= {0.195f, 0.195f, 0.195f, 1.0f};
		style->Colors[ImGuiCol_DockingPreview]			= {1.0f, 0.391f, 0.0f, 0.781f};
		style->Colors[ImGuiCol_DockingEmptyBg]			= {0.180f, 0.180f, 0.180f, 1.0f};
		style->Colors[ImGuiCol_PlotLines]				= {0.469f, 0.469f, 0.469f, 1.0f};
		style->Colors[ImGuiCol_PlotLinesHovered]		= {1.0f, 0.391f, 0.0f, 1.0f};
		style->Colors[ImGuiCol_PlotHistogram]			= {0.586f, 0.586f, 0.586f, 1.0f};
		style->Colors[ImGuiCol_PlotHistogramHovered]	= {1.0f, 0.391f, 0.0f, 1.0f};
		style->Colors[ImGuiCol_TextSelectedBg]			= {1.0f, 1.0f, 1.0f, 0.156f};
		style->Colors[ImGuiCol_DragDropTarget]			= {1.0f, 0.391f, 0.0f, 1.0f};
		style->Colors[ImGuiCol_NavHighlight]			= {1.0f, 0.391f, 0.0f, 1.0f};
		style->Colors[ImGuiCol_NavWindowingHighlight]	= {1.0f, 0.391f, 0.0f, 1.0f};
		style->Colors[ImGuiCol_NavWindowingDimBg]		= {0.0f, 0.0f, 0.0f, 0.586f};
		style->Colors[ImGuiCol_ModalWindowDimBg]		= {0.0f, 0.0f, 0.0f, 0.586f};

		mImGuiInput = new ImGuiInput(*mEventManager);

		mImGuiRenderer = new ImGuiRenderer("ImGuiRenderer");
		auto& renderGraph = mExternalTools->graphicsEngine->getRenderGraph();
		mImGuiRenderer->findInput("target")->connect( renderGraph.getNode("renderer2D")->findOutput("target") );
		renderGraph.addNode( std::unique_ptr<ImGuiRenderer>(mImGuiRenderer) );

		/**** Add the GUI components ****/
		mMenuBar = new MenuBar(*this);
		addPanel(new EntityPanel(*this));
		addPanel(new ComponentPanel(*this));
		addPanel(new RepositoryPanel(*this));
		addPanel(new SceneNodesPanel(*this));
		addPanel(new Gizmo(*this));

		/**** Create the Entity used for controlling the viewport ****/
		mViewportEntity = mEntityDatabase->addEntity();
		auto vTransforms = mEntityDatabase->emplaceComponent<se::app::TransformsComponent>(mViewportEntity);
		vTransforms->position = { 10.0, 8.0f,-6.0f };
		vTransforms->orientation = glm::quat(glm::vec3(glm::radians(-30.0f), glm::radians(110.0f), 0.0f));

		mEntityDatabase->addComponent<se::app::ScriptComponent>(mViewportEntity, std::make_unique<ViewportControl>());

		se::app::CameraComponent camera;
		camera.setPerspectiveProjection(glm::radians(kFOV), kWidth / static_cast<float>(kHeight), kZNear, kZFar);
		mEntityDatabase->addComponent(mViewportEntity, std::move(camera));

		mEventManager->publish(new se::app::ContainerEvent<se::app::Topic::Camera, se::app::Entity>(mViewportEntity));

		/**** Create the viewport grid ****/
		mGridEntity = mEntityDatabase->addEntity();

		mEntityDatabase->emplaceComponent<se::app::TransformsComponent>(mGridEntity);

		auto mesh = mEntityDatabase->emplaceComponent<se::app::MeshComponent>(mGridEntity);
		auto gridRawMesh = se::app::MeshLoader::createGridMesh("grid", 50, 100.0f);
		auto gridMeshSPtr = std::make_shared<se::graphics::Mesh>(se::app::MeshLoader::createGraphicsMesh(gridRawMesh));
		auto gridMesh = mRepository->insert(std::move(gridMeshSPtr), "gridMesh");
		std::size_t gridIndex = mesh->add(false, gridMesh, se::graphics::PrimitiveType::Line);

		std::shared_ptr<se::graphics::Program> program3DSPtr;
		auto result = se::app::ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragment3D.glsl", program3DSPtr);
		if (!result) {
			SOMBRA_ERROR_LOG << result.description();
			return;
		}
		auto program3D = mRepository->insert(std::move(program3DSPtr), "program3D");
		program3D.getResource().setPath("res/shaders/vertex3D.glsl||res/shaders/fragment3D.glsl");

		auto renderer = dynamic_cast<se::graphics::Renderer*>(mExternalTools->graphicsEngine->getRenderGraph().getNode("forwardRendererMesh"));
		auto stepGrid = mRepository->insert(std::make_shared<se::app::RenderableShaderStep>(*renderer), "stepGrid");
		stepGrid->addResource(program3D)
			.addBindable(std::make_shared<se::graphics::UniformVariableValue<glm::vec4>>("uColor", program3D.get(), glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)))
			.addBindable(std::make_shared<se::graphics::SetOperation>(se::graphics::Operation::Culling, false));
		auto shaderGrid = mRepository->insert(std::make_shared<se::app::RenderableShader>(*mEventManager), "shaderGrid");
		shaderGrid->addStep(stepGrid);
		mesh->addRenderableShader(gridIndex, shaderGrid);

		SOMBRA_INFO_LOG << "Editor created";
	}


	Editor::~Editor()
	{
		SOMBRA_INFO_LOG << "Destroying the Editor";

		if (mScene) {
			destroyScene();
		}

		if (mGridEntity != se::app::kNullEntity) { mEntityDatabase->removeEntity(mGridEntity); }
		if (mViewportEntity != se::app::kNullEntity) { mEntityDatabase->removeEntity(mViewportEntity); }

		for (IEditorPanel* panel : mPanels) {
			delete panel;
		}
		if (mMenuBar) { delete mMenuBar; }

		mImGuiRenderer->disconnect();
		mExternalTools->graphicsEngine->getRenderGraph().removeNode(mImGuiRenderer);
		if (mImGuiInput) { delete mImGuiInput; }
		if (mImGuiContext) { ImGui::DestroyContext(mImGuiContext); }

		if (mEventManager) { mEventManager->unsubscribe(this, se::app::Topic::Close); }

		SOMBRA_INFO_LOG << "Editor destroyed";
	}


	void Editor::createScene(const char* name)
	{
		SOMBRA_INFO_LOG << "Creating a Scene";

		mScene = new se::app::Scene(name, *this);

		SOMBRA_INFO_LOG << "Scene created";
	}


	void Editor::destroyScene()
	{
		SOMBRA_INFO_LOG << "Destroying the scene";

		mActiveEntity = se::app::kNullEntity;

		delete mScene;
		mScene = nullptr;

		SOMBRA_INFO_LOG << "Scene destroyed";
	}


	void Editor::addPanel(IEditorPanel* panel)
	{
		panel->setId(mPanelIdCount++);
		mPanels.push_back(panel);
	}


	bool Editor::notify(const se::app::IEvent& event)
	{
		return tryCall(&Editor::onCloseEvent, event);
	}

// Private functions
	void Editor::onUpdate(float deltaTime)
	{
		SOMBRA_DEBUG_LOG << "Init (" << deltaTime << ")";

		Application::onUpdate(deltaTime);

		ImGuiIO& io = ImGui::GetIO();
		io.DeltaTime = deltaTime;
	}


	void Editor::onRender()
	{
		ImGuiIO& io = ImGui::GetIO();
		const ImGuiViewport* viewport = ImGui::GetMainViewport();

		ImGuizmo::SetImGuiContext(mImGuiContext);
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		// Window
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar
			| ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground
			| ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoDocking
			| ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("EditorWindow", nullptr, windowFlags);
		ImGui::PopStyleVar();

		// Dockspace
		ImGui::DockSpace(ImGui::GetID("EditorDockSpace"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, io.DisplaySize.x, io.DisplaySize.y);

		// TODO: Remove ImGui demo window
		static bool show = true;
		ImGui::ShowDemoWindow(&show);

		mMenuBar->render();
		for (auto itPanel = mPanels.begin(); itPanel != mPanels.end();) {
			if ((*itPanel)->render()) {
				++itPanel;
			}
			else {
				itPanel = mPanels.erase(itPanel);
			}
		}

		ImGui::End();

		Application::onRender();
	}


	void Editor::onCloseEvent(const se::app::Event<se::app::Topic::Close>&)
	{
		stop();
	}

}
