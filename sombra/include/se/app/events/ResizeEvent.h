#ifndef RESIZE_EVENT_H
#define RESIZE_EVENT_H

#include "Event.h"

namespace se::app {

	/**
	 * Class ResizeEvent, its a generic event used for notifying of a resize
	 */
	template <Topic t>
	class ResizeEvent : public Event<t>
	{
	private:	// Attributes
		/** The new width */
		double mWidth;

		/** The new height */
		double mHeight;

	public:		// Functions
		/** Creates a new ResizeEvent
		 *
		 * @param	width the new width
		 * @param	height the new height */
		ResizeEvent(double width, double height) :
			mWidth(width), mHeight(height) {};

		/** @return	the new width */
		double getWidth() const { return mWidth; };

		/** @return	the new height */
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
			os	<< "{ kTopic : " << this->kTopic
				<< ", mWidth : " << mWidth << ", mHeight : " << mHeight << " }";
		};
	};


	using WindowResizeEvent = ResizeEvent<Topic::WindowResize>;
	using RendererResolutionEvent = ResizeEvent<Topic::RendererResolution>;

}

#endif		// RESIZE_EVENT_H
