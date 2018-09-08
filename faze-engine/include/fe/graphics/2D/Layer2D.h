#ifndef LAYER_2D_H
#define LAYER_2D_H

#include <vector>
#include "../ILayer.h"
#include "Renderer2D.h"

namespace fe { namespace graphics {

	class Renderable2D;


	/**
	 * Class Layer2D, it's a class that holds all the 2D elements that the
	 * Renderer2D must render
	 */
	class Layer2D : public ILayer
	{
	private:	// Attributes
		/** The Renderer used by the layer to render the Scene */
		Renderer2D mRenderer2D;

		/** The 3D renderables that the layer must render */
		std::vector<const Renderable2D*> mRenderable2Ds;

	public:		// Functions
		/** Creates a new Layer 3D */
		Layer2D() {};

		/** Class destructor */
		~Layer2D() {};

		/** Adds the given Renderable to the Layer so it will be rendered
		 *
		 * @param	renderable2D renderable2D a pointer to the Renderable2D to
		 *			add */
		void addRenderable2D(const Renderable2D* renderable2D);

		/** Removes the given Renderable from the Layer so it will no longer
		 * be rendered
		 *
		 * @param	renderable2D renderable2D a pointer to the Renderable2D to
		 *			remove */
		void removeRenderable2D(const Renderable2D* renderable2D);

		/** Draws the scene */
		void render() override;
	};

}}

#endif		// LAYER_2D_H
