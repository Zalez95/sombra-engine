#ifndef RENDERER_3D_H
#define RENDERER_3D_H

#include <queue>
#include <glm/glm.hpp>
#include "Program3D.h"
#include "Program3DSkinning.h"

namespace se::graphics {

	class Renderable3D;
	class Camera;
	struct PointLight;


	/**
	 * Class Renderer3D, it's a Forward Renderer used for rendering
	 * Renderable3Ds without skeletal animation
	 */
	class Renderer3D
	{
	private:	// Attributes
		/** The Program of the renderer */
		Program3D mProgram;

		/** The Program with skinning animation of the renderer */
		Program3DSkinning mSkinningProgram;

		/** The Renderable3Ds that we want to render */
		std::queue<const Renderable3D*> mRenderable3Ds;

		/** The Renderable3Ds that with skeletal animation we want to render */
		std::queue<const Renderable3D*> mSkinnedRenderable3Ds;

	public:		// Functions
		/** Creates a new Renderer3D */
		Renderer3D();

		/** Class destructor */
		~Renderer3D();

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
		 * @param	pointLights a vector with pointers to the lights that will
		 *			affect to the next renders
		 * @note	after calling this method the render queue will be empty */
		void render(
			const Camera* camera,
			const std::vector<const PointLight*>& pointLights
		);
	private:
		/** Sets the uniforms and other properties needed for rendering with the
		 * given material
		 *
		 * @param	material the material to use
		 * @param	program the program to use */
		void startMaterial(const Material& material, Program3D& program);

		/** Clears the properties setted for rendering with the given material
		 *
		 * @param	material the used material */
		void endMaterial(const Material& material);
	};

}

#endif		// RENDERER_3D_H
