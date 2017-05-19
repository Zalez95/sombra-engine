#ifndef SCENE_RENDERER_H
#define SCENE_RENDERER_H

#include <queue>
#include <glm/glm.hpp>
#include "SceneProgram.h"

namespace graphics {

	class Renderable3D;
	class PointLight;
	class Camera;


	/**
	 * Class SceneRenderer, it's a Forward Renderer used for rendering
	 * Renderable3Ds without skeletal animation
	 */
	class SceneRenderer
	{
	private:	// Attributes
		/** The Program of the renderer */
		SceneProgram mProgram;

		/** The projection matrix of the renderer that transforms from View
		 * Space to Projection Space */
		glm::mat4 mProjectionMatrix;

		/** The Renderables that we want to render */
		std::queue<const Renderable3D*> mRenderable3Ds;

	public:		// Functions
		/** Creates a new SceneRenderer and sets all the uniform locations
		 * for the renderer
		 *
		 * @param	projectionMatrix the projectionMatrix of the renderer */
		SceneRenderer(const glm::mat4& projectionMatrix) :
			mProjectionMatrix(projectionMatrix) {};

		/** Class destructor */
		~SceneRenderer() {};
		
		/** Sets the projection matrix */
		inline void setProjectionMatrix(const glm::mat4& projectionMatrix)
		{ mProjectionMatrix = projectionMatrix; };

		/** Submits the given Renderable3D to the queue of Renderable3Ds to
		 * render
		 * 
		 * @param	renderable3D a pointer to the Renderable3D that we want
		 *			to render */
		inline void submit(const Renderable3D* renderable3D)
		{ mRenderable3Ds.push(renderable3D); };

		/** Renders the Renderable3Ds that currently are in the render queue
		 * 
		 * @note	after calling this method the render queue will be empty
		 * @param	camera a pointer to the camera with which we will render
		 *			the scene
		 * @param	lights a vector with pointers to the lights that will
		 *			affect to the next renders */
		void render(
			const Camera* camera,
			const std::vector<const PointLight*>& pointLights
		);
	};

}

#endif		// SCENE_RENDERER_H
