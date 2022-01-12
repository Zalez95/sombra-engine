#ifndef GRAPHICS_ENGINE_H
#define GRAPHICS_ENGINE_H

#include <mutex>
#include "Renderable.h"
#include "RenderGraph.h"
#include "Context.h"

namespace se::graphics {

	/**
	 * Class GraphicsEngine. It prepares all the API context and provides the
	 * common interface used for rendering any kind of Renderable
	 */
	class GraphicsEngine
	{
	private:	// Attributes
		/** The graphics Context that holds the shared Bindables of the
		 * GraphicsEngine */
		Context mContext;

		/** The Renderables that the GraphicsEngine will render */
		std::vector<Renderable*> mRenderables;

		/** The RenderGraph used for drawing the Renderables */
		std::unique_ptr<RenderGraph> mRenderGraph;

		/** The mutex used for blocking the access to @see mRenderables
		 * and @see mRenderGraph */
		std::mutex mMutex;

	public:		// Functions
		/** Creates a new Graphics System
		 *
		 * @throw	runtime_error if failed to initialize GLEW
		 * @note	this function must be executed from the main graphics
		 *			thread */
		GraphicsEngine();

		/** Class destructor */
		~GraphicsEngine();

		/** @return	the Context of the GraphicsEngine */
		Context& getContext() { return mContext; };

		/** Sets the RenderGraph used for drawing the Renderables
		 *
		 * @param	renderGraph a pointer to the new RenderGraph of the
		 *			GraphicsEngine */
		void setRenderGraph(std::unique_ptr<RenderGraph> renderGraph);

		/** @return	a reference to the RenderGraph of the GraphicsEngine */
		const RenderGraph& getRenderGraph() { return *mRenderGraph; };

		/** Function used for accessing the RenderGraph in a thread safe way
		 *
		 * @param	callback the callback function that will be executed */
		template <typename F>
		void editRenderGraph(F&& callback)
		{ std::scoped_lock lck(mMutex); callback(*mRenderGraph); }

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

		/** Draws all the Renderables
		 * @note	this function must be executed from the main graphics
		 *			thread */
		void render();
	};

}

#endif		// GRAPHICS_ENGINE_H
