#ifndef RENDERER_3D_H
#define RENDERER_3D_H

#include <queue>
#include <glm/glm.hpp>
#include "Program3D.h"

namespace fe { namespace graphics {

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

		/** The projection matrix of the renderer that transforms from View
		 * Space to Projection Space */
		glm::mat4 mProjectionMatrix;

		/** The Renderable3Ds that we want to render */
		std::queue<const Renderable3D*> mRenderable3Ds;

	public:		// Functions
		/** Creates a new Renderer3D
		 *
		 * @param	projectionMatrix the projectionMatrix of the renderer */
		Renderer3D(const glm::mat4& projectionMatrix) :
			mProjectionMatrix(projectionMatrix) {};

		/** Class destructor */
		~Renderer3D() {};

		/** Sets the projection matrix of the Renderer
		 *
		 * @param	projectionMatrix the perspective matrix used to project
		 *			the 3D scene */
		inline void setProjectionMatrix(const glm::mat4& projectionMatrix)
		{ mProjectionMatrix = projectionMatrix; };

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

}}

#endif		// RENDERER_3D_H
