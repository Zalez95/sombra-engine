#ifndef RENDERER_3D_H
#define RENDERER_3D_H

#include <queue>
#include <glm/glm.hpp>
#include "Program3D.h"

namespace se::graphics {

	class Renderable3D;
	class PointLight;
	class Camera;


	/**
	 * Class Renderer3D, it's a Forward Renderer used for rendering
	 * Renderable3Ds without skeletal animation
	 */
	class Renderer3D
	{
	private:	// Attributes
		/** The Program of the renderer */
		Program3D mProgram;

		/** The Renderable3Ds that we want to render */
		std::queue<const Renderable3D*> mRenderable3Ds;

	public:		// Functions
		/** Submits the given Renderable3D to the queue of Renderable3Ds to
		 * render
		 *
		 * @param	renderable3D a pointer to the Renderable3D that we want
		 *			to render */
		void submit(const Renderable3D* renderable3D);

		/** Renders the Renderable3Ds that currently are in the render queue
		 *
		 * @param	camera a pointer to the Camera used to set the perspective
		 * 			from where we are going to render the scene
		 * @param	lights a vector with pointers to the lights that will
		 *			affect to the next renders
		 * @note	after calling this method the render queue will be empty */
		void render(
			const Camera* camera,
			const std::vector<const PointLight*>& pointLights
		);
	};

}

#endif		// RENDERER_3D_H
