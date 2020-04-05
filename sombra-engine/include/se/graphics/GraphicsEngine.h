#ifndef GRAPHICS_ENGINE_H
#define GRAPHICS_ENGINE_H

#include <vector>
#include <string>
#include "ILayer.h"

namespace se::graphics {

	/**
	 * Class GraphicsEngine. It prepares all the OpenGL data and holds all the
	 * Layers to render
	 */
	class GraphicsEngine
	{
	private:	// Attributes
		/** The Layers that the GraphicsEngine will render */
		std::vector<ILayer*> mLayers;

		/** The size of the layers viewports */
		glm::uvec2 mViewportSize;

	public:		// Functions
		/** Creates a new Graphics System
		 *
		 * @param	viewportSize the initial viewport size of the layers
		 * @throw	runtime_error if failed to initialize GLEW */
		GraphicsEngine(const glm::uvec2& viewportSize);

		/** @return	the OpenGL version info */
		std::string getGLInfo() const;

		/** Sets the viewport resolution
		 *
		 * @param	viewportSize the new size of the layer viewports */
		void setViewport(const glm::uvec2& viewportSize);

		/** Adds the given ILayer to the GraphicsEngine so it will
		 * be rendered in each render call
		 *
		 * @param	layer a pointer to the ILayer that we want
		 *			to add to the GraphicsEngine */
		void addLayer(ILayer* layer);

		/** Removes the given ILayer from the GraphicsEngine
		 *
		 * @param	layer a pointer to the ILayer that we want to remove from
		 *			the GraphicsEngine
		 * @note	you must call this function before deleting the
		 *			Layer */
		void removeLayer(ILayer* layer);

		/** Draws the scene */
		void render();
	};

}

#endif		// GRAPHICS_ENGINE_H
