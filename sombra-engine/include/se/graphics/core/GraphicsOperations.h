#ifndef GRAPHICS_OPERATIONS_H
#define GRAPHICS_OPERATIONS_H

#include <string>
#include <bitset>
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

		/** Returns the Viewport origin and its dimensions
		 *
		 * @param	x a reference to the origin of the viewport in the X axis
		 * @param	y a reference to the origin of the viewport in the Y axis
		 * @param	width a reference to the width of the viewport
		 * @param	height a reference to the height of the viewport */
		static void getViewport(
			int& x, int& y, std::size_t& width, std::size_t& height
		);

		/** Draws the VertexBuffers bound
		 *
		 * @param	primitive the type of primitive to draw
		 * @param	vertexCount the number of vertices in the VertexBuffer
		 *			bound */
		static void drawArrays(
			PrimitiveType primitive, std::size_t vertexCount
		);

		/** Draws the VertexBuffers bound using the IndexBuffer bound
		 *
		 * @param	primitive the type of primitive to draw
		 * @param	indexCount the number of indices in the IndexBuffer bound
		 * @param	indexType the type of indices in the IndexBuffer bound */
		static void drawIndexed(
			PrimitiveType primitive,
			std::size_t indexCount, TypeId indexType
		);

		/** Draws the VertexBuffers bound (instanced)
		 *
		 * @param	primitive the type of primitive to draw
		 * @param	vertexCount the number of vertices in the VertexBuffer bound
		 * @param	instanceCount the number of instances to draw */
		static void drawArraysInstanced(
			PrimitiveType primitive, std::size_t vertexCount,
			std::size_t instanceCount
		);

		/** Draws the VertexBuffers bound using the IndexBuffer bound
		 * (instanced)
		 *
		 * @param	primitive the type of primitive to draw
		 * @param	indexCount the number of indices in the IndexBuffer bound
		 * @param	indexType the type of indices in the IndexBuffer bound
		 * @param	instanceCount the number of instances to draw */
		static void drawIndexedInstanced(
			PrimitiveType primitive,
			std::size_t indexCount, TypeId indexType, std::size_t instanceCount
		);

		/** Clears the given buffers
		 *
		 * @param	mask a bit mask with the FrameBuffer buffers to clear */
		static void clear(const FrameBufferMask::Mask& mask);

		/** Enables or disables face culling
		 *
		 * @param	active if face culling should be enabled or not */
		static void setCulling(bool active);

		/** Sets the faces to cull
		 *
		 * @param	mode the faces to cull */
		static void setCullingMode(FaceMode mode);

		/** Enables or disables depth-testing
		 *
		 * @param	active if depth-testing should be enabled or not */
		static void setDepthTest(bool active);

		/** Enables or disables alpha-blending
		 *
		 * @param	active if alpha-blending should be enabled or not */
		static void setBlending(bool active);

		/** Enables or disables color components so the next draw operations
		 * won't write to the selected components
		 *
		 * @param	r if we want to write to the the red color component
		 * @param	g if we want to write to the the green color component
		 * @param	b if we want to write to the the blue color component
		 * @param	a if we want to write to the the alpha color component */
		static void setColorMask(bool r, bool g, bool b, bool a);

		/** Enables or disables writing to the depth buffer
		 *
		 * @param	active if the we should write to the depth buffer or not */
		static void setDepthMask(bool active);

		/** Adds a memory barrier for syncing image reads and writes */
		static void imageMemoryBarrier();
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
		virtual void bind() const override
		{ GraphicsOperations::setCulling(mActive); };

		/** Disables face culling if mActive is true, disables it otherwise */
		virtual void unbind() const override
		{ GraphicsOperations::setCulling(!mActive); };
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
		virtual void bind() const override
		{ GraphicsOperations::setDepthTest(mActive); };

		/** Disables depth-testing if mActive is true, disables it otherwise */
		virtual void unbind() const override
		{ GraphicsOperations::setDepthTest(!mActive); };
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
		virtual void bind() const override
		{ GraphicsOperations::setBlending(mActive); };

		/** Disables Blending if mActive is true, enables it otherwise */
		virtual void unbind() const override
		{ GraphicsOperations::setBlending(!mActive); };
	};


	/**
	 * Class DepthMaskOperation, it's a Bindable used for enabling or disabling
	 * writing to the depth buffer
	 */
	class DepthMaskOperation : public Bindable
	{
	private:	// Attributes
		/** If we want to write to the depth buffer on bind or not */
		bool mActive;

	public:		// Functions
		/** Creates a new DepthMaskOperation
		 *
		 * @param	active if we want to write to the depth buffer on bind or
		 *			not */
		DepthMaskOperation(bool active = true) : mActive(active) {};

		/** Enables writing to the depth buffer if mActive is true, disables
		 * it otherwise */
		virtual void bind() const override
		{ GraphicsOperations::setDepthMask(mActive); };

		/** Disables writing to the depth buffer if mActive is true, enables
		 * it otherwise */
		virtual void unbind() const override
		{ GraphicsOperations::setDepthMask(!mActive); };
	};

}

#endif		// GRAPHICS_OPERATIONS_H
