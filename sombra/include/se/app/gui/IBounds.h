#ifndef I_BOUNDS_H
#define I_BOUNDS_H

#include <glm/glm.hpp>

namespace se::app {

	/**
	 * Interface IBounds, its the interface that every bounds object must
	 * implement so the GUI components can check if the mouse is over them
	*/
	class IBounds
	{
	public:		// Functions
		/** Class destructor */
		virtual ~IBounds() = default;

		/** Creates a copy of the current IBounds object
		 *
		 * @return	a pointer to a Copy of the current IBounds object, the
		 *			caller of this function is responsible of clearing the
		 *			object after it's use */
		virtual IBounds* clone() const = 0;

		/** Sets the 2D position of the IBounds
		 *
		 * @param	position the new 2D position of the center of the IBounds */
		virtual void setPosition(const glm::vec2& position) = 0;

		/** Sets the 2D size of the IBounds
		 *
		 * @param	size the new 2D size of the IBounds */
		virtual void setSize(const glm::vec2& size) = 0;

		/** Checks if the given point is inside the IBounds
		 *
		 * @param	point the 2D point to check
		 * @return	true if the IBounds contain the point, false otherwise */
		virtual bool contains(const glm::vec2& point) const = 0;
	};

}

#endif		// I_BOUNDS_H
