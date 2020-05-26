#ifndef GRAPHICS_ENGINE_H
#define GRAPHICS_ENGINE_H

#include <mutex>
#include <glm/glm.hpp>
#include "RenderGraph.h"

namespace se::graphics {

	class Renderable;


	/** Struct GraphicsData, holds all the Configuration parameters of the
	 * GraphicsEngine */
	struct GraphicsData
	{
		/** The size of the layers viewports */
		glm::uvec2 viewportSize = glm::uvec2(0);
	};


	/**
	 * Class GraphicsEngine. It prepares all the API context and provides the
	 * common interface used for rendering any kind of Renderable
	 */
	class GraphicsEngine
	{
	private:	// Attributes
		/** The mutex used for blocking the access to the attributes */
		std::mutex mMutex;

		/** The size of the viewport */
		glm::uvec2 mViewportSize;

		/** The Renderables that the GraphicsEngine will render */
		std::vector<Renderable*> mRenderables;

		/** The RenderGraph used for drawing the Renderables */
		std::unique_ptr<RenderGraph> mRenderGraph;

	public:		// Functions
		/** Creates a new Graphics System
		 *
		 * @param	config the configuration parameters of the GraphicsEngine
		 * @throw	runtime_error if failed to initialize GLEW */
		GraphicsEngine(const GraphicsData& config);

		/** @return	the Graphics API version info and other limits */
		std::string getGraphicsInfo() const;

		/** Sets the viewport resolution
		 *
		 * @param	viewportSize the new size of the viewport */
		void setViewportSize(const glm::uvec2& viewportSize);

		/** @return	the viewport resolution */
		const glm::uvec2& getViewportSize() { return mViewportSize; };

		/** @return	the RenderGraph of the GraphicsEngine */
		RenderGraph& getRenderGraph() { return *mRenderGraph; };

		/** Adds the given Renderable to the GraphicsEngine so it will be
		 * rendered
		 *
		 * @param	renderable a pointer to the Renderable to add */
		void addRenderable(Renderable* renderable2D);

		/** Removes the given Renderable from the GraphicsEngine so it will no
		 * longer be rendered
		 *
		 * @param	renderable a pointer to the Renderable to remove */
		void removeRenderable(Renderable* renderable2D);

		/** Draws all the Renderables */
		void render();
	};

}

#endif		// GRAPHICS_ENGINE_H
