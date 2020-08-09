#include "se/app/gui/GUIManager.h"
#include "se/app/loaders/TechniqueLoader.h"

namespace se::app {

	GUIManager::GUIManager(
		EventManager& eventManager, GraphicsManager& graphicsManager, const glm::vec2& initialWindowSize
	) : mEventManager(eventManager), mGraphicsManager(graphicsManager)
	{
		mEventManager.subscribe(this, Topic::Resize);
		mEventManager.subscribe(this, Topic::Mouse);
		mRootComponent.setSize(initialWindowSize);

		auto program = TechniqueLoader::createProgram("res/shaders/vertex2D.glsl", nullptr, "res/shaders/fragment2D.glsl");
		if (!program) {
			throw std::runtime_error("program2D couldn't be created");
		}
		auto program2D = mGraphicsManager.getRepository().add<std::string, graphics::Program>("program2D", std::move(program));

		auto technique2D = std::make_unique<graphics::Technique>();
		technique2D->addPass( mGraphicsManager.createPass2D(program2D) );
		mGraphicsManager.getRepository().add<std::string, graphics::Technique>("technique2D", std::move(technique2D));
	}


	GUIManager::~GUIManager()
	{
		mEventManager.unsubscribe(this, Topic::Mouse);
		mEventManager.unsubscribe(this, Topic::Resize);
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
		mRootComponent.setSize({ static_cast<float>(event.getWidth()), static_cast<float>(event.getHeight()) });
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
