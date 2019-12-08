#ifndef RENDERER_PBR_H
#define RENDERER_PBR_H

#include <queue>
#include "ProgramPBR.h"
#include "ProgramPBRSkinning.h"
#include "Renderable3D.h"

namespace se::graphics {

	class Camera;
	struct ILight;


	/**
	 * Class RendererPBR, it's a Forward Renderer used for rendering the 3D
	 * scene with a PBR workflow
	 */
	class RendererPBR
	{
	private:	// Attributes
		/** The Program used for drawing the Renderable3Ds */
		ProgramPBR mProgramPBR;

		/** The Program used for drawing the Renderable3Ds with skinning
		 * animation */
		ProgramPBRSkinning mProgramPBRSkinning;

		/** The Renderable3Ds that we want to render */
		std::queue<const Renderable3D*> mRenderable3Ds;

		/** The Renderable3Ds that with skeletal animation we want to render */
		std::queue<const Renderable3D*> mSkinnedRenderable3Ds;

	public:		// Functions
		/** Creates a new RendererPBR */
		RendererPBR();

		/** Class destructor */
		~RendererPBR();

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
		 *			affect to the next render
		 * @note	after calling this method the render queue will be empty */
		void render(
			const Camera* camera,
			const std::vector<const ILight*>& lights
		);
	};

}

#endif		// RENDERER_PBR_H
