#ifndef RESIZE_EVENT_H
#define RESIZE_EVENT_H

#include "Event.h"

namespace se::app {

	/**
	 * Class ResizeEvent, its an event used for notifying of a window resize
	 * by the InputManager
	 */
	class ResizeEvent : public Event<Topic::Resize>
	{
	private:	// Attributes
		/** The window width */
		double mWidth;

		/** The window height */
		double mHeight;

	public:		// Functions
		/** Creates a new ResizeEvent
		 *
		 * @param	width the new window width
		 * @param	height the new window height */
		ResizeEvent(double width, double height) :
			mWidth(width), mHeight(height) {};

		/** @return	the window width */
		double getWidth() const { return mWidth; };

		/** @return	the window height */
		double getHeight() const { return mHeight; };

		/** Sets the ResizeEvent width
		 *
		 * @param	width the new ResizeEvent width */
		void setWidth(double width) { mWidth = width; };

		/** Sets the ResizeEvent height
		 *
		 * @param	height the new ResizeEvent height */
		void setHeight(double height) { mHeight = height; };
	protected:
		/** Appends the current ResizeEvent formated as text to the given
		 * ostream
		 *
		 * @param	os a reference to the ostream where we want to print the
		 *			current ResizeEvent */
		virtual void printTo(std::ostream& os) const override
		{
			os	<< "{ kTopic : " << kTopic
				<< ", mWidth : " << mWidth << ", mHeight : " << mHeight << " }";
		};
	};

}

#endif		// RESIZE_EVENT_H
