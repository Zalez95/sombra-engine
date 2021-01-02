#ifndef IMGUI_INPUT_H
#define IMGUI_INPUT_H

#include <se/app/events/EventManager.h>
#include <se/app/events/KeyEvent.h>
#include <se/app/events/MouseEvents.h>

namespace editor {

	/**
	 * Class ImGuiInput, acts as an adapter between the ImGui io input and the
	 * Application input events
	 */
	class ImGuiInput : public se::app::IEventListener
	{
	public:		// Attributes
		/** The EventManager used for notifying the Events */
		se::app::EventManager& mEventManager;

	public:		// Functions
		/** Creates a new ImGuiInput
		 *
		 * @param	eventManager the EventManager used for notifying the
		 *			Events */
		ImGuiInput(se::app::EventManager& eventManager);

		/** Class destructor */
		~ImGuiInput();

		/** Notifies the ImGuiInput of the given event
		 *
		 * @param	event the IEvent to notify */
		virtual void notify(const se::app::IEvent& event) override;
	private:
		/** Handles the given event
		 *
		 * @param	event the KeyEvent to handle */
		void onKeyEvent(const se::app::KeyEvent& event);

		/** Handles the given event
		 *
		 * @param	event the TextInputEvent to handle */
		void onTextInputEvent(const se::app::TextInputEvent& event);

		/** Handles the given event
		 *
		 * @param	event the MouseMoveEvent to handle */
		void onMouseMoveEvent(const se::app::MouseMoveEvent& event);

		/** Handles the given event
		 *
		 * @param	event the MouseScrollEvent to handle */
		void onMouseScrollEvent(const se::app::MouseScrollEvent& event);

		/** Handles the given event
		 *
		 * @param	event the MouseButtonEvent to handle */
		void onMouseButtonEvent(const se::app::MouseButtonEvent& event);
	};

}

#endif		// IMGUI_INPUT_H