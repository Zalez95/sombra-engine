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

		windowManager.onClose([this]() {
			std::scoped_lock lock(mMutex);
			mEventQueue.push_back(std::make_unique<Event<Topic::Close>>());
		});

		windowManager.onKey([this](int keyCode, window::ButtonState state) {
			std::scoped_lock lock(mMutex);
			KeyEvent::State keyState = (state == window::ButtonState::Pressed)? KeyEvent::State::Pressed :
				(state == window::ButtonState::Repeated)? KeyEvent::State::Repeated :
				KeyEvent::State::Released;
			mEventQueue.push_back(std::make_unique<KeyEvent>(keyCode, keyState));
		});

		windowManager.onTextInput([this](unsigned int codePoint) {
			std::scoped_lock lock(mMutex);
			mEventQueue.push_back(std::make_unique<TextInputEvent>(codePoint));
		});

		windowManager.onMouseButton([this](int buttonCode, window::ButtonState state) {
			std::scoped_lock lock(mMutex);
			MouseButtonEvent::State mbState = (state == window::ButtonState::Pressed)? MouseButtonEvent::State::Pressed :
				MouseButtonEvent::State::Released;
			mEventQueue.push_back(std::make_unique<MouseButtonEvent>(buttonCode, mbState));
		});

		windowManager.onMouseMove([this](double x, double y) {
			std::scoped_lock lock(mMutex);
			auto itEvent = std::find_if(
				mEventQueue.begin(), mEventQueue.end(),
				[](const auto& event) { return (event->getTopic() == Topic::MouseMove); }
			);
			if (itEvent != mEventQueue.end()) {
				auto event = static_cast<MouseMoveEvent*>(itEvent->get());
				event->setX(x);
				event->setY(y);
			}
			else {
				mEventQueue.push_back(std::make_unique<MouseMoveEvent>(x, y));
			}
		});

		windowManager.onScroll([this](double xOffset, double yOffset) {
			std::scoped_lock lock(mMutex);
			auto itEvent = std::find_if(
				mEventQueue.begin(), mEventQueue.end(),
				[](const auto& event) { return (event->getTopic() == Topic::MouseScroll); }
			);
			if (itEvent != mEventQueue.end()) {
				auto event = static_cast<MouseScrollEvent*>(itEvent->get());
				event->setXOffset(xOffset);
				event->setYOffset(yOffset);
			}
			else {
				mEventQueue.push_back(std::make_unique<MouseScrollEvent>(xOffset, yOffset));
			}
		});

		windowManager.onResize([this](double x, double y) {
			std::scoped_lock lock(mMutex);
			auto itEvent = std::find_if(
				mEventQueue.begin(), mEventQueue.end(),
				[](const auto& event) { return event->getTopic() == Topic::WindowResize; }
			);
			if (itEvent != mEventQueue.end()) {
				auto event = static_cast<WindowResizeEvent*>(itEvent->get());
				event->setWidth(x);
				event->setHeight(y);
			}
			else {
				mEventQueue.push_back(std::make_unique<WindowResizeEvent>(x, y));
			}
		});

		mApplication.getEventManager().subscribe(this, Topic::SetMousePos);
	}


	InputSystem::~InputSystem()
	{
		mApplication.getEventManager().unsubscribe(this, Topic::SetMousePos);
	}


	void InputSystem::update(float, float)
	{
		SOMBRA_DEBUG_LOG << "Updating the InputSystem";

		std::unique_lock lock(mMutex);
		SOMBRA_DEBUG_LOG << "EventQueue size = " << mEventQueue.size();

		auto& eventManager = mApplication.getEventManager();
		while (!mEventQueue.empty()) {
			auto currentEvent = std::move(mEventQueue.front());
			mEventQueue.pop_front();
			lock.unlock();

			eventManager.publish(std::move(currentEvent));

			lock.lock();
		}

		SOMBRA_DEBUG_LOG << "InputSystem updated";
	}

// Private functions
	void InputSystem::onSetMousePosEvent(const SetMousePosEvent& event)
	{
		SOMBRA_INFO_LOG << event;

		std::scoped_lock lock(mMutex);
		mApplication.getExternalTools().windowManager->setMousePosition(event.getX(), event.getY());
		mEventQueue.push_back(std::make_unique<MouseMoveEvent>(event.getX(), event.getY()));
	}

}
