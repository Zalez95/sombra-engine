#include "se/app/gui/GUIManager.h"
#include "se/app/loaders/TechniqueLoader.h"

namespace se::app {

	GUIManager::GUIManager(
		EventManager& eventManager, graphics::GraphicsEngine& graphicsEngine,
		utils::Repository& repository, const glm::vec2& initialWindowSize
	) : mEventManager(eventManager), mGraphicsEngine(graphicsEngine), mRepository(repository)
	{
		mEventManager.subscribe(this, Topic::Resize);
		mEventManager.subscribe(this, Topic::Mouse);
		mRootComponent.setSize(initialWindowSize);
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
