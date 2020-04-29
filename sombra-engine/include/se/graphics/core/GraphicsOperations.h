#ifndef GRAPHICS_OPERATIONS_H
#define GRAPHICS_OPERATIONS_H

#include <string>
#include <functional>
#include "Bindable.h"
#include "Constants.h"

namespace se::graphics {

	/**
	 * Class GraphicsOperations, holds all the functions used for interacting
	 * with the underlying Graphics API.
	 */
	class GraphicsOperations
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


	/**
	 * Class CullingOperation, it's a Bindable used for enabling or disabling
	 * face culling
	 */
	class CullingOperation : public Bindable
	{
	private:	// Attributes
		/** If the disables culling should be enabled or not */
		bool mActive;

	public:		// Functions
		/** Creates a new CullingOperation
		 *
		 * @param	active if the disables culling should be enabled or not */
		CullingOperation(bool active = true) : mActive(active) {};

		/** Enables face culling if mActive is true, disables it otherwise */
		virtual void bind() const override { setCulling(mActive); };

		/** Disables face culling if mActive is true, disables it otherwise */
		virtual void unbind() const override { setCulling(!mActive); };
	private:
		/** Enables or disables face culling
		 *
		 * @param	active if face culling should be enabled or not */
		static void setCulling(bool active);
	};


	/**
	 * Class DepthTestOperation, it's a Bindable used for enabling or disabling
	 * depth-testing
	 */
	class DepthTestOperation : public Bindable
	{
	private:	// Attributes
		/** If the depth-testing should be enabled or not */
		bool mActive;

	public:		// Functions
		/** Creates a new DepthTestOperation
		 *
		 * @param	active if the depth-testing should be enabled or not */
		DepthTestOperation(bool active = true) : mActive(active) {};

		/** Enables depth-testing if mActive is true, disables it otherwise */
		virtual void bind() const override { setDepthTest(mActive); };

		/** Disables depth-testing if mActive is true, disables it otherwise */
		virtual void unbind() const override { setDepthTest(!mActive); };
	private:
		/** Enables or disables depth-testing
		 *
		 * @param	active if depth-testing should be enabled or not */
		static void setDepthTest(bool active);
	};


	/**
	 * Class BlendingOperation, it's a Bindable used for enabling or disabling
	 * alpha-blending
	 */
	class BlendingOperation : public Bindable
	{
	private:	// Attributes
		/** If the alpha-blending should be enabled or not */
		bool mActive;

	public:		// Functions
		/** Creates a new BlendingOperation
		 *
		 * @param	active if the alpha-blending should be enabled or not */
		BlendingOperation(bool active = true) : mActive(active) {};

		/** Enables Blending if mActive is true, disables it otherwise */
		virtual void bind() const override { setBlending(mActive); };

		/** Disables Blending if mActive is true, enables it otherwise */
		virtual void unbind() const override { setBlending(!mActive); };
	private:
		/** Enables or disables alpha-blending
		 *
		 * @param	active if alpha-blending should be enabled or not */
		static void setBlending(bool active);
	};

}

#endif		// GRAPHICS_OPERATIONS_H
