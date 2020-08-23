#include <glm/gtc/matrix_transform.hpp>
#include "se/utils/Repository.h"
#include "se/utils/StringUtils.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/graphics/Pass.h"
#include "se/graphics/Technique.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/graphics/2D/Renderer2D.h"
#include "se/app/gui/GUIManager.h"
#include "se/app/Application.h"
#include "se/app/loaders/TechniqueLoader.h"

namespace se::app {

	GUIManager::GUIManager(Application& application, const glm::vec2& initialWindowSize) : mApplication(application)
	{
		mApplication.getEventManager().subscribe(this, Topic::Resize);
		mApplication.getEventManager().subscribe(this, Topic::Mouse);

		if (!mApplication.getRepository().find<std::string, graphics::Program>("technique2D")) {
			auto renderer = dynamic_cast<graphics::Renderer*>(mApplication.getExternalTools().graphicsEngine->getRenderGraph().getNode("renderer2D"));

			auto program = mApplication.getRepository().find<std::string, graphics::Program>("program2D");
			if (!program) {
				program = TechniqueLoader::createProgram("res/shaders/vertex2D.glsl", nullptr, "res/shaders/fragment2D.glsl");
				mApplication.getRepository().add(std::string("program2D"), program);
			}

			mProjectionMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", *program,
				glm::ortho(0.0f, initialWindowSize.x, initialWindowSize.y, 0.0f, -1.0f, 1.0f)
			);

			auto pass = std::make_shared<graphics::Pass>(*renderer);
			pass->addBindable(program)
				.addBindable(std::make_shared<graphics::BlendingOperation>(true))
				.addBindable(std::make_shared<graphics::DepthTestOperation>(false))
				.addBindable(mProjectionMatrix);

			for (int i = 0; i < static_cast<int>(graphics::Renderer2D::kMaxTextures); ++i) {
				utils::ArrayStreambuf<char, 64> aStreambuf;
				std::ostream(&aStreambuf) << "uTextures[" << i << "]";
				pass->addBindable(std::make_shared<graphics::UniformVariableValue<int>>(aStreambuf.data(), *program, i));
			}

			auto technique2D = std::make_shared<graphics::Technique>();
			technique2D->addPass(pass);
			mApplication.getRepository().add(std::string("technique2D"), std::move(technique2D));
		}

		mRootComponent.setSize(initialWindowSize);
	}


	GUIManager::~GUIManager()
	{
		mApplication.getEventManager().unsubscribe(this, Topic::Mouse);
		mApplication.getEventManager().unsubscribe(this, Topic::Resize);
	}


	void GUIManager::add(IComponent* component, const Anchor& anchor, const Proportions& proportions)
	{
		mRootComponent.add(component, anchor, proportions);
	}


	void GUIManager::remove(IComponent* component)
	{
		mRootComponent.remove(component);
	}


	void GUIManager::notify(const IEvent& event)
	{
		tryCall(&GUIManager::onResizeEvent, event);
		tryCall(&GUIManager::onMouseEvent, event);
	}

// Private functions
	void GUIManager::onResizeEvent(const ResizeEvent& event)
	{
		float width = static_cast<float>(event.getWidth()),
			height = static_cast<float>(event.getHeight());

		if (mProjectionMatrix) {
			mProjectionMatrix->setValue(glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f));
		}

		mRootComponent.setSize({ width, height });
	}


	void GUIManager::onMouseEvent(const MouseEvent& event)
	{
		switch (event.getType()) {
			case MouseEvent::Type::Move:
				mRootComponent.onHover(static_cast<const MouseMoveEvent&>(event));
				break;
			case MouseEvent::Type::ButtonPressed:
				mRootComponent.onClick(static_cast<const MouseButtonEvent&>(event));
				break;
			case MouseEvent::Type::ButtonReleased:
				mRootComponent.onRelease(static_cast<const MouseButtonEvent&>(event));
				break;
			default:
				break;
		}
	}

}
