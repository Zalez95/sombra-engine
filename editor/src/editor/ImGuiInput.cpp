#include <imgui.h>
#include <se/window/KeyCodes.h>
#include <se/window/MouseButtonCodes.h>
#include "ImGuiInput.h"

namespace editor {

	ImGuiInput::ImGuiInput(se::app::EventManager& eventManager) : mEventManager(eventManager)
	{
		mEventManager.subscribe(this, se::app::Topic::Key);
		mEventManager.subscribe(this, se::app::Topic::TextInput);
		mEventManager.subscribe(this, se::app::Topic::MouseMove);
		mEventManager.subscribe(this, se::app::Topic::MouseScroll);
		mEventManager.subscribe(this, se::app::Topic::MouseButton);
		mEventManager.subscribe(this, se::app::Topic::WindowResize);

		ImGuiIO& io = ImGui::GetIO();
		io.KeyMap[ImGuiKey_Tab] = SE_KEY_TAB;
		io.KeyMap[ImGuiKey_LeftArrow] = SE_KEY_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = SE_KEY_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = SE_KEY_UP;
		io.KeyMap[ImGuiKey_DownArrow] = SE_KEY_DOWN;
		io.KeyMap[ImGuiKey_PageUp] = SE_KEY_PAGE_UP;
		io.KeyMap[ImGuiKey_PageDown] = SE_KEY_PAGE_DOWN;
		io.KeyMap[ImGuiKey_Home] = SE_KEY_HOME;
		io.KeyMap[ImGuiKey_End] = SE_KEY_END;
		io.KeyMap[ImGuiKey_Insert] = SE_KEY_INSERT;
		io.KeyMap[ImGuiKey_Delete] = SE_KEY_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = SE_KEY_BACKSPACE;
		io.KeyMap[ImGuiKey_Space] = SE_KEY_SPACE;
		io.KeyMap[ImGuiKey_Enter] = SE_KEY_ENTER;
		io.KeyMap[ImGuiKey_Escape] = SE_KEY_ESCAPE;
		io.KeyMap[ImGuiKey_A] = SE_KEY_A;
		io.KeyMap[ImGuiKey_C] = SE_KEY_C;
		io.KeyMap[ImGuiKey_V] = SE_KEY_V;
		io.KeyMap[ImGuiKey_X] = SE_KEY_X;
		io.KeyMap[ImGuiKey_Y] = SE_KEY_Y;
		io.KeyMap[ImGuiKey_Z] = SE_KEY_Z;
	}


	ImGuiInput::~ImGuiInput()
	{
		mEventManager.unsubscribe(this, se::app::Topic::WindowResize);
		mEventManager.unsubscribe(this, se::app::Topic::MouseButton);
		mEventManager.unsubscribe(this, se::app::Topic::MouseScroll);
		mEventManager.unsubscribe(this, se::app::Topic::MouseMove);
		mEventManager.unsubscribe(this, se::app::Topic::TextInput);
		mEventManager.unsubscribe(this, se::app::Topic::Key);
	}


	bool ImGuiInput::notify(const se::app::IEvent& event)
	{
		return tryCall(&ImGuiInput::onKeyEvent, event)
			|| tryCall(&ImGuiInput::onTextInputEvent, event)
			|| tryCall(&ImGuiInput::onMouseMoveEvent, event)
			|| tryCall(&ImGuiInput::onMouseScrollEvent, event)
			|| tryCall(&ImGuiInput::onMouseButtonEvent, event)
			|| tryCall(&ImGuiInput::onWindowResizeEvent, event);
	}

// Private functions
	void ImGuiInput::onKeyEvent(const se::app::KeyEvent& event)
	{
		ImGuiIO& io = ImGui::GetIO();
		if (event.getState() == se::app::KeyEvent::State::Pressed) {
			io.KeysDown[event.getKeyCode()] = true;

			io.KeyCtrl = io.KeysDown[SE_KEY_LEFT_CONTROL] || io.KeysDown[SE_KEY_RIGHT_CONTROL];
			io.KeyShift = io.KeysDown[SE_KEY_LEFT_SHIFT] || io.KeysDown[SE_KEY_RIGHT_SHIFT];
			io.KeyAlt = io.KeysDown[SE_KEY_LEFT_ALT] || io.KeysDown[SE_KEY_RIGHT_ALT];
			io.KeySuper = io.KeysDown[SE_KEY_LEFT_SUPER] || io.KeysDown[SE_KEY_RIGHT_SUPER];
		}
		else {
			io.KeysDown[event.getKeyCode()] = false;
		}
	}


	void ImGuiInput::onTextInputEvent(const se::app::TextInputEvent& event)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddInputCharacter(event.getCodePoint());
	}


	void ImGuiInput::onMouseMoveEvent(const se::app::MouseMoveEvent& event)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2(static_cast<float>(event.getX()), static_cast<float>(event.getY()));
	}


	void ImGuiInput::onMouseScrollEvent(const se::app::MouseScrollEvent& event)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseWheelH += static_cast<float>(event.getXOffset());
		io.MouseWheel += static_cast<float>(event.getYOffset());
	}


	void ImGuiInput::onMouseButtonEvent(const se::app::MouseButtonEvent& event)
	{
		ImGuiIO& io = ImGui::GetIO();
		switch (event.getButtonCode()) {
			case SE_MOUSE_BUTTON_LEFT:
				io.MouseDown[ImGuiMouseButton_Left] = (event.getState() == se::app::MouseButtonEvent::State::Pressed);
				break;
			case SE_MOUSE_BUTTON_RIGHT:
				io.MouseDown[ImGuiMouseButton_Right] = (event.getState() == se::app::MouseButtonEvent::State::Pressed);
				break;
			case SE_MOUSE_BUTTON_MIDDLE:
				io.MouseDown[ImGuiMouseButton_Middle] = (event.getState() == se::app::MouseButtonEvent::State::Pressed);
				break;
		}
	}


	void ImGuiInput::onWindowResizeEvent(const se::app::WindowResizeEvent& event)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(static_cast<float>(event.getWidth()), static_cast<float>(event.getHeight()));
		mEventManager.publish(std::make_unique<se::app::RendererResolutionEvent>(event.getWidth(), event.getHeight()));
	}

}
