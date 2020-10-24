#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "IBounds.h"

namespace se::app {

	/**
	 * Class Rectangle, it's a IBounds object with the shape of a 2D rectangle
	*/
	class Rectangle : public IBounds
	{
	private:	// Attributes
		/** The position of the center of the Rectangle */
		glm::vec2 mPosition;

		/** The size of the Rectangle */
		glm::vec2 mSize;

	public:		// Functions
		/** Creates a copy of the current Rectangle object
		 *
		 * @return	a pointer to a Copy of the current Rectangle object, the
		 *			caller of this function is responsible of clearing the
		 *			object after it's use */
		virtual IBounds* clone() const override
		{ return new Rectangle(*this); };

		/** Sets the 2D position of the Rectangle
		 *
		 * @param	position the new 2D position of the top-left corner of the
		 *			Rectangle */
		virtual void setPosition(const glm::vec2& position) override
		{ mPosition = position; };

		/** Sets the 2D size of the Rectangle
		 *
		 * @param	size the new 2D size of the Rectangle */
		virtual void setSize(const glm::vec2& size) override
		{ mSize = size; };

		/** Checks if the given point is inside the Rectangle
		 *
		 * @param	point the 2D point to check
		 * @return	true if the Rectangle contain the point, false otherwise */
		virtual bool contains(const glm::vec2& point) const override
		{
			glm::vec2 bl = mPosition - mSize / 2.0f;
			glm::vec2 tr = mPosition + mSize / 2.0f;
			return glm::all(glm::greaterThanEqual(point, bl))
				&& glm::all(glm::lessThanEqual(point, tr));
		};
	};

}

#endif		// RECTANGLE_H
