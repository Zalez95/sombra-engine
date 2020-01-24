#include "se/app/gui/GUIManager.h"

namespace se::app {

	GUIManager::GUIManager(EventManager& eventManager) : mEventManager(eventManager)
	{
		mEventManager.subscribe(this, Topic::Resize);
		mEventManager.subscribe(this, Topic::Mouse);
	}


	GUIManager::~GUIManager()
	{
		mEventManager.unsubscribe(this, Topic::Mouse);
		mEventManager.unsubscribe(this, Topic::Resize);
	}


	void GUIManager::add(IComponent* component)
	{
		mRootComponent.add(component);
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
	void GUIManager::onResizeEvent(const ResizeEvent& /*event*/)
	{
		// TODO:
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
