#ifndef RENDERER_3D_H
#define RENDERER_3D_H

#include <vector>

namespace se::graphics {

	class Renderable3D;
	class Step3D;


	/**
	 * Class Renderer3D, it's a Batch Renderer used for rendering 3D
	 * graphics elements.
	 */
	class Renderer3D
	{
	private:	// Nested types
		using RenderableStepPair = std::pair<Renderable3D*, Step3D*>;

	private:	// Attributes
		/** The submited Renderable3Des that are going to be drawn */
		std::vector<RenderableStepPair> mRenderQueue;

	public:		// Functions
		/** Submits the given Renderable3D for rendering
		 *
		 * @param	renderable the Renderable3D to submit for rendering
		 * @param	step the Step3D with which the Renderable3D will be drawn */
		void submit(Renderable3D& renderable, Step3D& step);

		/** Renders the Renderable3Ds that are currently in the render queue */
		void render();
	};

}

#endif		// RENDERER_3D_H
