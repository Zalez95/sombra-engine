#ifndef MOUSE_EVENTS_H
#define MOUSE_EVENTS_H

#include "Event.h"

namespace se::app {

	/**
	 * Class MousePositionEvent, it's an Event that holds the mouse position
	 */
	template <Topic t>
	class MousePositionEvent : public Event<t>
	{
	private:	// Attributes
		/** The mouse X position relative to the left edge */
		double mX;

		/** The mouse Y position relative to the top edge */
		double mY;

	public:		// Functions
		/** Creates a new MousePositionEvent
		 *
		 * @param	x the mouse X position
		 * @param	y the mouse Y position */
		MousePositionEvent(double x, double y) : mX(x), mY(y) {};

		/** @return	the mouse X position */
		double getX() const { return mX; };

		/** @return	the mouse Y position */
		double getY() const { return mY; };

		/** Sets the mouse X position
		 *
		 * @param	x the new mouse X position */
		void setX(double x) { mX = x; };

		/** Sets the mouse Y position
		 *
		 * @param	y the new mouse Y position */
		void setY(double y) { mY = y; };
	private:
		/** Appends the current MouseMoveEvent formated as text to the given
		 * ostream
		 *
		 * @param	os a reference to the ostream where we want to print the
		 *			current MouseMoveEvent */
		virtual void printTo(std::ostream& os) const override
		{
			os	<< "{ kTopic : " << t
				<< ", mX : " << mX << ", mY : " << mY << " }";
		};
	};


	using MouseMoveEvent = MousePositionEvent<Topic::MouseMove>;
	using SetMousePosEvent = MousePositionEvent<Topic::SetMousePos>;


	/**
	 * Class MouseScrollEvent, it's an Event used for notifying of a mouse
	 * scroll state change by the InputManager
	 */
	class MouseScrollEvent : public Event<Topic::MouseScroll>
	{
	private:	// Attributes
		/** The scroll X offset */
		double mXOffset;

		/** The scroll Y offset */
		double mYOffset;

	public:		// Functions
		/** Creates a new MouseScrollEvent
		 *
		 * @param	xOffset the scroll X offset
		 * @param	yOffset the scroll Y offset */
		MouseScrollEvent(double xOffset, double yOffset) :
			mXOffset(xOffset), mYOffset(yOffset) {};

		/** @return	the scroll X offset */
		double getXOffset() const { return mXOffset; };

		/** @return	the scroll Y offset */
		double getYOffset() const { return mYOffset; };

		/** Sets the scroll X offset
		 *
		 * @param	xOffset the new scroll X offset */
		void setXOffset(double xOffset) { mXOffset = xOffset; };

		/** Sets the scroll Y offset
		 *
		 * @param	yOffset the new scroll Y offset */
		void setYOffset(double yOffset) { mYOffset = yOffset; };
	private:
		/** Appends the current MouseScrollEvent formated as text to the given
		 * ostream
		 *
		 * @param	os a reference to the ostream where we want to print the
		 *			current MouseScrollEvent */
		virtual void printTo(std::ostream& os) const override
		{
			os	<< "{ kTopic : " << kTopic
				<< ", mXOffset : " << mXOffset
				<< ", mYOffset : " << mYOffset << " }";
		};
	};


	/**
	 * Class MouseButtonEvent, it's an Event used for notifying of a mouse
	 * button state change by the InputManager
	 */
	class MouseButtonEvent : public Event<Topic::MouseButton>
	{
	public:		// Nested types
		/** The different state in which a button can be */
		enum class State { Pressed, Released };

	private:	// Attributes
		/** The button code of the MouseButtonEvent */
		int mButtonCode;

		/** The state of the button */
		State mState;

	public:		// Functions
		/** Creates a new MouseButtonEvent
		 *
		 * @param	buttonCode the button code
		 * @param	state the state of the button */
		MouseButtonEvent(int buttonCode, State state) :
			mButtonCode(buttonCode), mState(state) {};

		/** @return	the button code of the MouseButtonEvent */
		int getButtonCode() const { return mButtonCode; };

		/** @return	the state of the button */
		State getState() const { return mState; };
	private:
		/** Prints the state to the given stream
		 *
		 * @param	os the stream to print to
		 * @param	s the state to print
		 * @return	the stream */
		friend constexpr std::ostream& operator<<(
			std::ostream& os, const State& s
		) {
			switch (s) {
				case State::Pressed:	return os << "State::Pressed";
				case State::Released:	return os << "State::Released";
				default:				return os;
			}
		};

		/** Appends the current MouseButtonEvent formated as text to the given
		 * ostream
		 *
		 * @param	os a reference to the ostream where we want to print the
		 *			current MouseButtonEvent */
		virtual void printTo(std::ostream& os) const override
		{
			os	<< "{ kTopic : " << kTopic
				<< ", mButtonCode : " << mButtonCode
				<< ", mState : " << mState << " }";
		};
	};

}

#endif		// MOUSE_EVENTS_H
