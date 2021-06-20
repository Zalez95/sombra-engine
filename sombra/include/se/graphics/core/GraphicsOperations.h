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
		 * @param	x the origin (lower-left) of the viewport in the X axis
		 * @param	y the origin (lower-left) of the viewport in the Y axis
		 * @param	width the width of the viewport
		 * @param	height the height of the viewport */
		static void setViewport(
			int x, int y, std::size_t width, std::size_t height
		);

		/** Returns the Viewport origin and its dimensions
		 *
		 * @param	x a reference to the origin (lower-left) of the viewport in
		 *			the X axis
		 * @param	y a reference to the origin (lower-left) of the viewport in
		 *			the Y axis
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
		 * @param	indexType the type of indices in the IndexBuffer bound
		 * @param	offset the byte offset from the start of the buffer that
		 *			contains the vertex data where indices start */
		static void drawIndexed(
			PrimitiveType primitive,
			std::size_t indexCount, TypeId indexType, std::size_t offset = 0
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
		 * @param	offset the byte offset from the start of the buffer that
		 *			contains the vertex data where indices start
		 * @param	instanceCount the number of instances to draw */
		static void drawIndexedInstanced(
			PrimitiveType primitive,
			std::size_t indexCount, TypeId indexType, std::size_t offset = 0,
			std::size_t instanceCount = 1
		);

		/** Clears the given buffers
		 *
		 * @param	mask a bit mask with the FrameBuffer buffers to clear */
		static void clear(const FrameBufferMask::Mask& mask);

		/** Enables or disables the given operation
		 *
		 * @param	active if the operation should be enabled or not */
		static void setOperation(Operation operation, bool active);

		/** @return	true if the operation is enabled, false otherwise */
		static bool hasOperation(Operation operation);

		/** Sets the faces to cull
		 *
		 * @param	mode the faces to cull */
		static void setCullingMode(FaceMode mode);

		/** Sets the Scissor-box origin and its dimensions
		 *
		 * @param	x the origin (lower-left) of the scissor box in the X axis
		 * @param	y the origin (lower-left) of the scissor box in the Y axis
		 * @param	width the width of the scissor box
		 * @param	height the height of the scissor box */
		static void setScissorBox(
			int x, int y, std::size_t width, std::size_t height
		);

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

		/** @return	true if writting to the depth buffer is enabled or not */
		static bool hasDepthMask();

		/** Adds a memory barrier for syncing image reads and writes */
		static void imageMemoryBarrier();
	};


	/**
	 * Class SetOperation, it's a Bindable used for enabling or disabling
	 * an operation
	 */
	class SetOperation : public Bindable
	{
	private:	// Attributes
		/** The operation to enable/disable */
		Operation mOperation;

		/** If the operation should be enabled or disabled */
		bool mActive;

		/** The previous state of the operation */
		mutable bool mLastActive;

	public:		// Functions
		/** Creates a new SetOperation
		 *
		 * @param	operation the operation to enable/disable
		 * @param	active if the operation should be enabled or not */
		SetOperation(Operation operation, bool active = true) :
			mOperation(operation), mActive(active), mLastActive(false) {};

		/** @return	the Operation to enable/disable */
		Operation getOperation() const { return mOperation; };

		/** @return	true if the operation should be enabled, false otherwise */
		bool isEnabled() const { return mActive; };

		/** Enables/Disables the operation on bind
		 *
		 * @param	active if we want to enable the operation on bind or not */
		void setEnabled(bool active = true) { mActive = active; };

		/** @copydoc Bindable::clone() */
		virtual std::unique_ptr<Bindable> clone() const override
		{ return std::make_unique<SetOperation>(*this); };

		/** Enables the Operation if mActive is true, disables it otherwise */
		virtual void bind() const override;

		/** Recovers the previous state of the Operation after calling
		 * @see bind */
		virtual void unbind() const override;
	};


	/**
	 * Class SetDepthMask, it's a Bindable used for enabling or disabling
	 * writing to the depth buffer
	 */
	class SetDepthMask : public Bindable
	{
	private:	// Attributes
		/** If we want to write to the depth buffer on bind or not */
		bool mActive;

		/** The previous state of the operation */
		mutable bool mLastActive;

	public:		// Functions
		/** Creates a new SetDepthMask
		 *
		 * @param	active if we want to write to the depth buffer on bind or
		 *			not */
		SetDepthMask(bool active = true) :
			mActive(active), mLastActive(false) {};

		/** @return	true if we can write to the depth buffer on bind, false
		 *			otherwise */
		bool isEnabled() const { return mActive; };

		/** Enables/Disables the depth buffer on bind
		 *
		 * @param	active if we want to write to the depth buffer on bind or
		 *			not */
		void setEnabled(bool active = true) { mActive = active; };

		/** @copydoc Bindable::clone() */
		virtual std::unique_ptr<Bindable> clone() const override
		{ return std::make_unique<SetDepthMask>(*this); };

		/** Enables writing to the depth buffer if mActive is true, disables
		 * it otherwise */
		virtual void bind() const override;

		/** Recovers the previous state of the Depth Mask after calling
		 * @see bind */
		virtual void unbind() const override;
	};

}

#endif		// GRAPHICS_OPERATIONS_H
