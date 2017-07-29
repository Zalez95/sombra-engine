#ifndef GRAPHICS_SYSTEM_H
#define GRAPHICS_SYSTEM_H

#include <vector>
#include "2D/Layer2D.h"
#include "3D/Layer3D.h"

namespace graphics {

	/**
	 * Class GraphicsSystem
	 */
	class GraphicsSystem
	{
	private:	// Attributes
		/** The Layer used by the Graphics System to render all the 2D
		 * elements */
		Layer2D mLayer2D;

		/** The Layer used by the Graphics System to render all the 3D
		 * elements */
		Layer3D mLayer3D;

	public:		// Functions
		/** Creates a new Graphics System */
		GraphicsSystem();

		/** Class destructor */
		~GraphicsSystem() {};

		/** @return a pointer to the Graphics System's 2D layer */
		Layer2D* getLayer2D() { return &mLayer2D; };

		/** @return a pointer to the Graphics System's 3D layer */
		Layer3D* getLayer3D() { return &mLayer3D; };

		/** Draws the scene */
		void render();
	};

}

#endif		// GRAPHICS_SYSTEM_H
