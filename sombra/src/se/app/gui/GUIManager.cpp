#include <glm/gtc/matrix_transform.hpp>
#include "se/app/Repository.h"
#include "se/utils/StringUtils.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/graphics/Pass.h"
#include "se/graphics/Technique.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/graphics/2D/Renderer2D.h"
#include "se/app/gui/GUIManager.h"
#include "se/app/Application.h"
#include "se/app/io/ShaderLoader.h"

namespace se::app {

	GUIManager::GUIManager(Application& application, const glm::vec2& initialWindowSize) : mApplication(application)
	{
		mApplication.getEventManager().subscribe(this, Topic::WindowResize);
		mApplication.getEventManager().subscribe(this, Topic::MouseMove);
		mApplication.getEventManager().subscribe(this, Topic::MouseButton);

		mTechnique = mApplication.getRepository().findByName<graphics::Technique>("technique2D");
		if (!mTechnique) {
			auto& context = mApplication.getExternalTools().graphicsEngine->getContext();
			auto& renderGraph = mApplication.getExternalTools().graphicsEngine->getRenderGraph();

			ProgramRef programRef;
			mProgramResource = mApplication.getRepository().findByName<ProgramRef>("program2D");
			if (mProgramResource) {
				programRef = *mProgramResource;
			}
			else {
				auto result = ShaderLoader::createProgram("res/shaders/vertex2D.glsl", nullptr, "res/shaders/fragment2D.glsl", context, programRef);
				if (!result) {
					SOMBRA_ERROR_LOG << result.description();
					return;
				}
				mProgramResource = mApplication.getRepository().insert(std::make_shared<ProgramRef>(programRef), "program2D");
			}

			glm::mat4 projectionMatrix = glm::ortho(0.0f, initialWindowSize.x, initialWindowSize.y, 0.0f, -1.0f, 1.0f);
			mProjectionMatrix = context.create<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", projectionMatrix)
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); });

			auto renderer = dynamic_cast<graphics::Renderer*>(renderGraph.getNode("renderer2D"));
			auto pass = std::make_shared<graphics::Pass>(*renderer);
			pass->addBindable(*mProgramResource)
				.addBindable(context.create<graphics::SetOperation>(graphics::Operation::Blending, true))
				.addBindable(context.create<graphics::SetOperation>(graphics::Operation::DepthTest, false))
				.addBindable(mProjectionMatrix);

			for (int i = 0; i < static_cast<int>(graphics::Renderer2D::kMaxTextures); ++i) {
				pass->addBindable(
					context.create<graphics::UniformVariableValue<int>>("uTextures[" + std::to_string(i) + "]", i)
						.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
				);
			}

			auto technique2D = std::make_shared<graphics::Technique>();
			technique2D->addPass(pass);
			mTechnique = mApplication.getRepository().insert(std::move(technique2D), "technique2D");
		}

		mRootComponent.setSize(initialWindowSize);
	}


	GUIManager::~GUIManager()
	{
		mApplication.getEventManager().unsubscribe(this, Topic::MouseButton);
		mApplication.getEventManager().unsubscribe(this, Topic::MouseMove);
		mApplication.getEventManager().unsubscribe(this, Topic::WindowResize);
	}


	void GUIManager::add(IComponent* component, const Anchor& anchor, const Proportions& proportions)
	{
		mRootComponent.add(component, anchor, proportions);
	}


	void GUIManager::remove(IComponent* component)
	{
		mRootComponent.remove(component);
	}


	bool GUIManager::notify(const IEvent& event)
	{
		return tryCall(&GUIManager::onWindowResizeEvent, event)
			|| tryCall(&GUIManager::onMouseMoveEvent, event)
			|| tryCall(&GUIManager::onMouseButtonEvent, event);
	}

// Private functions
	void GUIManager::onWindowResizeEvent(const WindowResizeEvent& event)
	{
		float width = static_cast<float>(event.getWidth()),
			height = static_cast<float>(event.getHeight());

		if (mProjectionMatrix) {
			mProjectionMatrix.edit([=](auto& uniform) { uniform.setValue(glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f)); });
		}

		mRootComponent.setSize({ width, height });
	}


	void GUIManager::onMouseMoveEvent(const MouseMoveEvent& event)
	{
		mRootComponent.onHover(static_cast<const MouseMoveEvent&>(event));
	}


	void GUIManager::onMouseButtonEvent(const MouseButtonEvent& event)
	{
		switch (event.getState()) {
			case MouseButtonEvent::State::Pressed:
				mRootComponent.onClick(static_cast<const MouseButtonEvent&>(event));
				break;
			case MouseButtonEvent::State::Released:
				mRootComponent.onRelease(static_cast<const MouseButtonEvent&>(event));
				break;
			default:
				break;
		}
	}

}
