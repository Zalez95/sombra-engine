#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <string>
#include "Constants.h"

namespace se::graphics {

	/**
	 * Class Graphics, holds all the functions used for interactuate with the
	 * underlying Graphics API.
	 */
	class Graphics
	{
	public:
		/** Initializes the Graphics contexts
		 *
		 * @return	true on success, false otherwise */
		static bool init();

		/** @return	the Graphics API version info and other limits */
		static std::string getGraphicsInfo();

		/** Sets the Viewport origin and its dimensions
		 *
		 * @param	x the origin of the viewport in the X axis
		 * @param	y the origin of the viewport in the Y axis
		 * @param	width the width of the viewport
		 * @param	height the height of the viewport */
		static void setViewport(
			int x, int y, std::size_t width, std::size_t height
		);

		/** Enables or disables face culling
		 *
		 * @param	active if face culling should be enabled or not */
		static void setCulling(bool active = true);

		/** Enables or disables depth-testing
		 *
		 * @param	active if depth-testing should be enabled or not */
		static void setDepthTest(bool active = true);

		/** Enables or disables alpha-blending
		 *
		 * @param	active if alpha-blending should be enabled or not */
		static void setBlending(bool active = true);

		/** Draws the VertexBuffers bound
		 *
		 * @param	primitive the type of primitive to draw
		 * @param	indexCount the number of indices in the IndexBuffer bound
		 * @param	indexType the type of indices in the IndexBuffer bound */
		static void drawIndexed(
			PrimitiveType primitive,
			std::size_t indexCount, TypeId indexType);

		/** Draws the VertexBuffers bound
		 *
		 * @param	primitive the type of primitive to draw
		 * @param	indexCount the number of vertices in the VertexBuffer
		 *			bound */
		static void drawArrays(
			PrimitiveType primitive, std::size_t vertexCount
		);

		/** Draws the VertexBuffers bound (instanced)
		 *
		 * @param	primitive the type of primitive to draw
		 * @param	indexCount the number of vertices in the VertexBuffer bound
		 * @param	instanceCount the number of instances to draw */
		static void drawArraysInstanced(
			PrimitiveType primitive, std::size_t vertexCount,
			std::size_t instanceCount
		);

		/** Clears the given buffers
		 *
		 * @param	color if the color buffer should be cleared or not
		 * @param	depth if the depth buffer should be cleared or not
		 * @param	stencil if the stencil buffer should be cleared or not */
		static void clear(
			bool color = false, bool depth = false, bool stencil = false
		);
	};

}

#endif		// GRAPHICS_H
