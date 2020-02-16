#ifndef GRAPHICS_SYSTEM_H
#define GRAPHICS_SYSTEM_H

#include <vector>
#include <string>
#include "ILayer.h"

namespace se::graphics {

	/**
	 * Class GraphicsSystem. It prepares all the OpenGL data and holds all the
	 * Layers to render
	 */
	class GraphicsSystem
	{
	private:	// Attributes
		/** The Layers that the GraphicsSystem will render */
		std::vector<ILayer*> mLayers;

		/** The size of the layers viewports */
		glm::uvec2 mViewportSize;

	public:		// Functions
		/** Creates a new Graphics System
		 *
		 * @param	viewportSize the initial viewport size of the layers
		 * @throw	runtime_error if failed to initialize GLEW */
		GraphicsSystem(const glm::uvec2& viewportSize);

		/** @return	the OpenGL version info */
		std::string getGLInfo() const;

		/** Sets the viewport resolution
		 *
		 * @param	viewportSize the new size of the layer viewports */
		void setViewport(const glm::uvec2& viewportSize);

		/** Adds the given ILayer to the GraphicsSystem so it will
		 * be rendered in each render call
		 *
		 * @param	layer a pointer to the ILayer that we want
		 *			to add to the GraphicsSystem */
		void addLayer(ILayer* layer);

		/** Removes the given ILayer from the GraphicsSystem
		 *
		 * @param	layer a pointer to the ILayer that we want to remove from
		 *			the GraphicsSystem
		 * @note	you must call this function before deleting the
		 *			Layer */
		void removeLayer(ILayer* layer);

		/** Draws the scene */
		void render();
	};

}

#endif		// GRAPHICS_SYSTEM_H
