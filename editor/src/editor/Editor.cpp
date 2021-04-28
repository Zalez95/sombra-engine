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
#include <se/app/EntityDatabase.h>
#include <se/app/RenderableShader.h>
#include <se/app/TransformsComponent.h>
#include <se/app/CameraComponent.h>
#include <se/app/MeshComponent.h>
#include <se/app/events/ContainerEvent.h>
#include "Editor.h"
#include "ViewportControl.h"

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
		mMenuBar(nullptr), mEntityPanel(nullptr), mComponentPanel(nullptr),
		mRepositoryPanel(nullptr), mSceneNodesPanel(nullptr), mGizmo(nullptr),
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

		// Create the ImGui context and renderer
		IMGUI_CHECKVERSION();
		mImGuiContext = ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(kWidth, kHeight);
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		mImGuiInput = new ImGuiInput(*mEventManager);

		mImGuiRenderer = new ImGuiRenderer("ImGuiRenderer");
		auto& renderGraph = mExternalTools->graphicsEngine->getRenderGraph();
		mImGuiRenderer->findInput("target")->connect( renderGraph.getNode("renderer2D")->findOutput("target") );
		renderGraph.addNode( std::unique_ptr<ImGuiRenderer>(mImGuiRenderer) );

		// Add the GUI components
		mMenuBar = new MenuBar(*this);
		mEntityPanel = new EntityPanel(*this);
		mComponentPanel = new ComponentPanel(*this);
		mRepositoryPanel = new RepositoryPanel(*this);
		mSceneNodesPanel = new SceneNodesPanel(*this);
		mGizmo = new Gizmo(*this);

		// Create the Entity used for controlling the viewport
		mViewportEntity = mEntityDatabase->addEntity();
		auto vTransforms = mEntityDatabase->emplaceComponent<se::app::TransformsComponent>(mViewportEntity);
		vTransforms->position = { 10.0, 8.0f,-6.0f };
		vTransforms->orientation = glm::quat(glm::vec3(glm::radians(-30.0f), glm::radians(110.0f), 0.0f));

		mEntityDatabase->addComponent<se::app::ScriptComponent>(mViewportEntity, std::make_unique<ViewportControl>());

		se::app::CameraComponent camera;
		camera.setPerspectiveProjection(glm::radians(kFOV), kWidth / static_cast<float>(kHeight), kZNear, kZFar);
		mEntityDatabase->addComponent(mViewportEntity, std::move(camera));

		mEventManager->publish(new se::app::ContainerEvent<se::app::Topic::Camera, se::app::Entity>(mViewportEntity));

		// Create the viewport grid
		mGridEntity = mEntityDatabase->addEntity();

		mEntityDatabase->emplaceComponent<se::app::TransformsComponent>(mGridEntity);

		auto mesh = mEntityDatabase->emplaceComponent<se::app::MeshComponent>(mGridEntity, *mEventManager, mGridEntity);
		auto gridRawMesh = se::app::MeshLoader::createGridMesh("grid", 50, 100.0f);
		auto gridMesh = std::make_shared<se::graphics::Mesh>(se::app::MeshLoader::createGraphicsMesh(gridRawMesh));
		std::size_t gridIndex = mesh->add(false, gridMesh, se::graphics::PrimitiveType::Line);

		std::shared_ptr<se::graphics::Program> program;
		auto result = se::app::ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragment3D.glsl", program);
		if (!result) {
			SOMBRA_ERROR_LOG << result.description();
			return;
		}

		auto renderer = dynamic_cast<se::graphics::Renderer*>(mExternalTools->graphicsEngine->getRenderGraph().getNode("forwardRendererMesh"));
		auto pass = std::make_shared<se::graphics::Pass>(*renderer);
		pass->addBindable(program)
			.addBindable(std::make_shared<se::graphics::UniformVariableValue<glm::vec4>>("uColor", program, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)))
			.addBindable(std::make_shared<se::graphics::SetOperation>(se::graphics::Operation::Culling, false));
		auto rShader = std::make_shared<se::app::RenderableShader>(*mEventManager);
		rShader->addPass(pass);
		mesh->addRenderableShader(gridIndex, rShader);

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

		if (mGizmo) { delete mGizmo; }
		if (mSceneNodesPanel) { delete mSceneNodesPanel; }
		if (mRepositoryPanel) { delete mRepositoryPanel; }
		if (mComponentPanel) { delete mComponentPanel; }
		if (mEntityPanel) { delete mEntityPanel; }
		if (mMenuBar) { delete mMenuBar; }

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


	void Editor::notify(const se::app::IEvent& event)
	{
		tryCall(&Editor::onCloseEvent, event);
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
		mEntityPanel->render();
		mComponentPanel->render();
		mRepositoryPanel->render();
		mSceneNodesPanel->render();
		mGizmo->render();

		ImGui::End();

		Application::onRender();
	}


	void Editor::onCloseEvent(const se::app::Event<se::app::Topic::Close>&)
	{
		stop();
	}

}
