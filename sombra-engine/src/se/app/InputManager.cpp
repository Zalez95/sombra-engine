#include <algorithm>
#include "se/utils/Log.h"
#include "se/app/InputManager.h"
#include "se/app/Entity.h"
#include "se/app/events/KeyEvent.h"
#include "se/app/events/ResizeEvent.h"
#include "se/app/events/MouseEvent.h"

namespace se::app {

	InputManager::InputManager(window::WindowSystem& windowSystem, EventManager& eventManager) :
		mWindowSystem(windowSystem), mEventManager(eventManager)
	{
		mWindowSystem.onKey([&](int keyCode, window::ButtonState state) {
			KeyEvent::State keyState = (state == window::ButtonState::Pressed)? KeyEvent::State::Pressed :
				(state == window::ButtonState::Repeated)? KeyEvent::State::Repeated :
				KeyEvent::State::Released;
			mEventQueue.push_back(new KeyEvent(keyCode, keyState));
		});

		mWindowSystem.onMouseButton([&](int buttonCode, window::ButtonState state) {
			MouseEvent::Type type = (state == window::ButtonState::Pressed)? MouseEvent::Type::ButtonPressed :
				MouseEvent::Type::ButtonReleased;
			mEventQueue.push_back(new MouseButtonEvent(buttonCode, type));
		});

		mWindowSystem.onMouseMove([&](double x, double y) {
			auto itEvent = std::find_if(
				mEventQueue.begin(), mEventQueue.end(),
				[](const IEvent* event) {
					return (event->getTopic() == Topic::Mouse)
						&& (static_cast<const MouseEvent*>(event)->getType() == MouseEvent::Type::Move);
				}
			);
			if (itEvent != mEventQueue.end()) {
				auto event = static_cast<MouseMoveEvent*>(*itEvent);
				event->setX(x);
				event->setY(y);
			}
			else {
				mEventQueue.push_back(new MouseMoveEvent(x, y));
			}
		});

		mWindowSystem.onScroll([&](double x, double y) {
			auto itEvent = std::find_if(
				mEventQueue.begin(), mEventQueue.end(),
				[](const IEvent* event) {
					return (event->getTopic() == Topic::Mouse)
						&& (static_cast<const MouseEvent*>(event)->getType() == MouseEvent::Type::Scroll);
				}
			);
			if (itEvent != mEventQueue.end()) {
				auto event = static_cast<MouseScrollEvent*>(*itEvent);
				event->setX(x);
				event->setY(y);
			}
			else {
				mEventQueue.push_back(new MouseScrollEvent(x, y));
			}
		});

		mWindowSystem.onResize([&](double x, double y) {
			auto itEvent = std::find_if(
				mEventQueue.begin(), mEventQueue.end(),
				[](const IEvent* event) { return event->getTopic() == Topic::Resize; }
			);
			if (itEvent != mEventQueue.end()) {
				auto event = static_cast<ResizeEvent*>(*itEvent);
				event->setWidth(x);
				event->setHeight(y);
			}
			else {
				mEventQueue.push_back(new ResizeEvent(x, y));
			}
		});
	}


	void InputManager::update()
	{
		SOMBRA_INFO_LOG << "Updating the InputManager. EventQueue size = " << mEventQueue.size();

		while (!mEventQueue.empty()) {
			IEvent* currentEvent = mEventQueue.front();
			mEventQueue.pop_front();

			mEventManager.publish(currentEvent);
		}

		SOMBRA_INFO_LOG << "InputManager updated";
	}

}
