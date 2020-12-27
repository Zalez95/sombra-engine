#ifndef RENDERABLE_2D_H
#define RENDERABLE_2D_H

#include "../Renderable.h"

namespace se::graphics {

	class Renderer2D;


	/**
	 * Class Renderable2D, it's a 2D Renderable that can be drawn with a
	 * Technique
	 */
	class Renderable2D : public Renderable
	{
	private:	// Attributes
		/** The z-index used for drawing the Renderable2D on top of other
		 * Renderable2Ds. The smaller the value the further back it will
		 * appear. */
		unsigned char mZIndex;

	public:		// Functions
		/** Creates a new Renderable2D */
		Renderable2D() : mZIndex(0) {};
		Renderable2D(const Renderable2D& other) = default;
		Renderable2D(Renderable2D&& other) = default;

		/** Class destructor */
		virtual ~Renderable2D() = default;

		/** Assignment operator */
		Renderable2D& operator=(const Renderable2D& other) = default;
		Renderable2D& operator=(Renderable2D&& other) = default;

		/** @return	the z-index of the Renderable2D */
		unsigned char getZIndex() const { return mZIndex; };

		/** Sets the z-index of the Renderable2D
		 *
		 * @param	zIndex the new z-index of the Renderable2D */
		virtual void setZIndex(unsigned char zIndex) { mZIndex = zIndex; };

		/** Submits the vertices of the current Renderable2D to the given
		 * Renderer2D
		 *
		 * @param	renderer the renderer where the Renderable2D vertices will
		 *			be submitted */
		virtual void submitVertices(Renderer2D& renderer) const = 0;
	};

}

#endif		// RENDERABLE_2D_H
