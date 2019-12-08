#ifndef RENDERER_SKY_H
#define RENDERER_SKY_H

#include "Program3D.h"

namespace se::graphics {

	class Camera;
	class Renderable3D;


	/**
	 * Class RendererSky, it's a Forward Renderer used for rendering the 3D sky
	 */
	class RendererSky
	{
	private:	// Attributes
		/** The Program used for drawing the sky */
		Program3D mProgram;

	public:		// Functions
		/** Creates a new RendererSky */
		RendererSky();

		/** Class destructor */
		~RendererSky();

		/** Renders the given Sky
		 *
		 * @param	camera a pointer to the Camera used to set the perspective
		 * 			from where we are going to render the scene
		 * @param	sky the Sky mesh and its properties to render */
		void render(const Camera* camera, const Renderable3D& sky);
	};

}

#endif		// RENDERER_SKY_H
