#ifndef VIEWPORT_CONTROL_H
#define VIEWPORT_CONTROL_H

#include <se/app/Entity.h>
#include <se/app/Application.h>
#include <se/app/events/EventManager.h>
#include <se/app/events/KeyEvent.h>
#include <se/app/events/MouseEvents.h>

namespace editor {

	/**
	 * Class ViewportControl, Implements Blender-like viewport controls for a
	 * Entity Camera
	 */
	class ViewportControl : public se::app::IEventListener
	{
	private:	// Attributes
		static constexpr float kMoveSpeed		= 25.0f;
		static constexpr float kRotationSpeed	= 10.0f;
		static constexpr float kPitchLimit		= 0.05f;

		/** The Application that holds the EntityDatabase and systems */
		se::app::Application& mApplication;

		/** The controlled Entity */
		se::app::Entity mEntity;

		/** The pressed keys */
		bool mCtrlPressed, mShiftPressed, mAltPressed;

		/** The last two mouse locations, the frist one is the newest one */
		glm::vec2 mLastMousePositions[2];

		/** The type of movement that is going to be used */
		bool mZoomActive, mMoveActive, mOrbitActive;

		/** The current zoom value */
		float mZoom;

	public:		// Functions
		/** Creates a new ViewportControl
		 *
		 * @param	application the Application that holds the EntityDatabase
		 *			and systems
		 * @param	entity the controlled Entity, it must have a transforms
		 *			component */
		ViewportControl(
			se::app::Application& application, se::app::Entity entity
		);

		/** Class destructor */
		~ViewportControl();

		/** Updates the Entity location based on the user input */
		void update();

		/** Notifies the PlayerController of the given event
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
		 * @param	event the MouseEventMove to handle */
		void onMouseMoveEvent(const se::app::MouseMoveEvent& event);

		/** Handles the given event
		 *
		 * @param	event the MouseEventButton to handle */
		void onMouseButtonEvent(const se::app::MouseButtonEvent& event);
	};

}

#endif		// VIEWPORT_CONTROL_H
