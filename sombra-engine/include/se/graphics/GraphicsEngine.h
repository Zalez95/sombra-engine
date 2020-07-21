#ifndef GRAPHICS_ENGINE_H
#define GRAPHICS_ENGINE_H

#include <mutex>
#include "Renderable.h"
#include "RenderGraph.h"

namespace se::graphics {

	/**
	 * Class GraphicsEngine. It prepares all the API context and provides the
	 * common interface used for rendering any kind of Renderable
	 */
	class GraphicsEngine
	{
	private:	// Attributes
		/** The mutex used for blocking the access to the attributes */
		std::mutex mMutex;

		/** The Renderables that the GraphicsEngine will render */
		std::vector<Renderable*> mRenderables;

		/** The RenderGraph used for drawing the Renderables */
		std::unique_ptr<RenderGraph> mRenderGraph;

	public:		// Functions
		/** Creates a new Graphics System
		 *
		 * @throw	runtime_error if failed to initialize GLEW */
		GraphicsEngine();

		/** @return	the RenderGraph of the GraphicsEngine */
		RenderGraph& getRenderGraph() { return *mRenderGraph; };

		/** Adds the given Renderable to the GraphicsEngine so it will be
		 * rendered
		 *
		 * @param	renderable a pointer to the Renderable to add */
		void addRenderable(Renderable* renderable);

		/** Removes the given Renderable from the GraphicsEngine so it will no
		 * longer be rendered
		 *
		 * @param	renderable a pointer to the Renderable to remove */
		void removeRenderable(Renderable* renderable);

		/** Draws all the Renderables */
		void render();
	};

}

#endif		// GRAPHICS_ENGINE_H
