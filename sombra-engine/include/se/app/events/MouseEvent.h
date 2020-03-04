#ifndef MOUSE_EVENT_H
#define MOUSE_EVENT_H

#include "Event.h"

namespace se::app {

	/**
	 * Class MouseEvent, it's an event used for notifying of a mouse event
	 * by the InputManager
	 */
	class MouseEvent : public Event<Topic::Mouse>
	{
	public:		// Nested types
		/** The type of mouse event */
		enum class Type
		{
			ButtonPressed,
			ButtonReleased,
			Move,
			Scroll
		};

	public:		// Functions
		/** Class destructor */
		~MouseEvent() = default;

		/** @return	the MouseEvent type */
		virtual Type getType() const = 0;
	protected:
		/** Prints the type to the given stream
		 *
		 * @param	os the stream to print to
		 * @param	t the type to print
		 * @return	the stream */
		friend constexpr std::ostream& operator<<(
			std::ostream& os, const Type& t
		) {
			switch (t) {
				case Type::ButtonPressed:	return os << "Type::ButtonPressed";
				case Type::ButtonReleased:	return os << "Type::ButtonReleased";
				case Type::Move:			return os << "Type::Move";
				case Type::Scroll:			return os << "Type::Scroll";
				default:					return os;
			}
		};
	};


	/**
	 * Class MouseMoveEvent, it's a MouseEvent used for notifying of a mouse
	 * movement by the InputManager
	*/
	class MouseMoveEvent : public MouseEvent
	{
	private:	// Attributes
		/** The mouse X position */
		double mX;

		/** The mouse Y position */
		double mY;

	public:		// Functions
		/** Creates a new MouseMoveEvent
		 *
		 * @param	x the mouse X position
		 * @param	y the mouse Y position */
		MouseMoveEvent(double x, double y) : mX(x), mY(y) {};

		/** @return	the MouseEvent type */
		virtual Type getType() const override { return Type::Move; };

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
		virtual void printTo(std::ostream& os) const
		{
			os	<< "{ kTopic : " << kTopic << ", type : " << getType()
				<< ", mX : " << mX << ", mY : " << mY << " }";
		};
	};


	/**
	 * Class MouseScrollEvent, it's a MouseEvent used for notifying of a mouse
	 * scroll state change by the InputManager
	 */
	class MouseScrollEvent : public MouseEvent
	{
	private:	// Attributes
		/** The scroll X position */
		double mX;

		/** The scroll Y position */
		double mY;

	public:		// Functions
		/** Creates a new MouseScrollEvent
		 *
		 * @param	x the scroll X position
		 * @param	y the scroll Y position */
		MouseScrollEvent(double x, double y) : mX(x), mY(y) {};

		/** @return	the MouseEvent type */
		virtual Type getType() const override { return Type::Scroll; };

		/** @return	the scroll X position */
		double getX() const { return mX; };

		/** @return	the scroll Y position */
		double getY() const { return mY; };

		/** Sets the scroll X position
		 *
		 * @param	x the new scroll X position */
		void setX(double x) { mX = x; };

		/** Sets the scroll Y position
		 *
		 * @param	y the new scroll Y position */
		void setY(double y) { mY = y; };
	private:
		/** Appends the current MouseScrollEvent formated as text to the given
		 * ostream
		 *
		 * @param	os a reference to the ostream where we want to print the
		 *			current MouseScrollEvent */
		virtual void printTo(std::ostream& os) const
		{
			os	<< "{ kTopic : " << kTopic << ", type : " << getType()
				<< ", mX : " << mX << ", mY : " << mY << " }";
		};
	};


	/**
	 * Class MouseButtonEvent, it's a MouseEvent used for notifying of a mouse
	 * button state change by the InputManager
	 */
	class MouseButtonEvent : public MouseEvent
	{
	private:	// Attributes
		/** The button code of the MouseButtonEvent */
		int mButtonCode;

		/** The state of the button */
		Type mState;

	public:		// Functions
		/** Creates a new MouseButtonEvent
		 *
		 * @param	buttonCode the button code
		 * @param	state the state of the button */
		MouseButtonEvent(int buttonCode, Type state) :
			mButtonCode(buttonCode), mState(state) {};

		/** @return	the MouseEvent type */
		virtual Type getType() const override { return mState; };

		/** @return	the button code of the MouseButtonEvent */
		int getButtonCode() const { return mButtonCode; };
	private:
		/** Appends the current MouseButtonEvent formated as text to the given
		 * ostream
		 *
		 * @param	os a reference to the ostream where we want to print the
		 *			current MouseButtonEvent */
		virtual void printTo(std::ostream& os) const
		{
			os	<< "{ kTopic : " << kTopic << ", type : " << getType()
				<< ", mButtonCode : " << mButtonCode
				<< ", mState : " << mState << " }";
		};
	};

}

#endif		// MOUSE_EVENT_H
