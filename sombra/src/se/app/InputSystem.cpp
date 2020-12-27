#include <algorithm>
#include "se/utils/Log.h"
#include "se/window/WindowManager.h"
#include "se/app/InputSystem.h"
#include "se/app/Application.h"
#include "se/app/events/KeyEvent.h"
#include "se/app/events/ResizeEvent.h"
#include "se/app/events/MouseEvents.h"

namespace se::app {

	InputSystem::InputSystem(Application& application) :
		ISystem(application.getEntityDatabase()), mApplication(application)
	{
		auto& windowManager = *mApplication.getExternalTools().windowManager;

		windowManager.onClose([&]() {
			mEventQueue.push_back(new Event<Topic::Close>());
		});

		windowManager.onKey([&](int keyCode, window::ButtonState state) {
			KeyEvent::State keyState = (state == window::ButtonState::Pressed)? KeyEvent::State::Pressed :
				(state == window::ButtonState::Repeated)? KeyEvent::State::Repeated :
				KeyEvent::State::Released;
			mEventQueue.push_back(new KeyEvent(keyCode, keyState));
		});

		windowManager.onTextInput([&](unsigned int codePoint) {
			mEventQueue.push_back(new TextInputEvent(codePoint));
		});

		windowManager.onMouseButton([&](int buttonCode, window::ButtonState state) {
			MouseButtonEvent::State mbState = (state == window::ButtonState::Pressed)? MouseButtonEvent::State::Pressed :
				MouseButtonEvent::State::Released;
			mEventQueue.push_back(new MouseButtonEvent(buttonCode, mbState));
		});

		windowManager.onMouseMove([&](double x, double y) {
			auto itEvent = std::find_if(
				mEventQueue.begin(), mEventQueue.end(),
				[](const IEvent* event) { return (event->getTopic() == Topic::MouseMove); }
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

		windowManager.onScroll([&](double xOffset, double yOffset) {
			auto itEvent = std::find_if(
				mEventQueue.begin(), mEventQueue.end(),
				[](const IEvent* event) { return (event->getTopic() == Topic::MouseScroll); }
			);
			if (itEvent != mEventQueue.end()) {
				auto event = static_cast<MouseScrollEvent*>(*itEvent);
				event->setXOffset(xOffset);
				event->setYOffset(yOffset);
			}
			else {
				mEventQueue.push_back(new MouseScrollEvent(xOffset, yOffset));
			}
		});

		windowManager.onResize([&](double x, double y) {
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


	void InputSystem::update()
	{
		SOMBRA_INFO_LOG << "Updating the InputSystem. EventQueue size = " << mEventQueue.size();

		auto& eventManager = mApplication.getEventManager();
		while (!mEventQueue.empty()) {
			IEvent* currentEvent = mEventQueue.front();
			mEventQueue.pop_front();

			eventManager.publish(currentEvent);
		}

		SOMBRA_INFO_LOG << "InputSystem updated";
	}

}
