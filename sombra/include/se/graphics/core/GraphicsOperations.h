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

		/** Changes the alpha blend equation to use
		 *
		 * @param	equation the new blend equation */
		static void setBlendEquation(BlendEquation equation);

		/** Changes the alpha blend functions to use
		 *
		 * @param	sourceFactor specifies how the source blending factors
		 *			are computed
		 * @param	destinationFactor specifies how the destination blending
		 *			factors are computed */
		static void setBlendFunction(
			BlendFunction sourceFactor, BlendFunction destinationFactor
		);

		/** Changes the current stencil function to use
		 *
		 * @param	function the new Stencil function
		 * @param	referenceValue the reference value used for comparing
		 *			during the stencil test
		 * @param	mask a mask of bits that will be ANDed with both the
		 *			referenceValue and the stored stencil one */
		static void setStencilFunction(
			StencilFunction function, int referenceValue, unsigned int mask
		);

		/** Sets the actions to take when the stencil test is done
		 *
		 * @param	face the face should be updated
		 * @param	stencilFailAction the action to take when the stencil test
		 *			fails
		 * @param	depthFailAction the action to take when the stencil test
		 *			passes but the depth test fails
		 * @param	passAction the action to take when the stencil and depth
		 *			tests passes successfully */
		static void setStencilAction(
			FaceMode face, StencilAction stencilFailAction,
			StencilAction depthFailAction, StencilAction passAction
		);

		/** Sets the faces to cull
		 *
		 * @param	mode the faces to cull */
		static void setCullingMode(FaceMode mode);

		/** Enables or disables the Wireframe draw mode
		 *
		 * @param	active if the wireframe should be enabled or disabled */
		static void setWireframe(bool active);

		/** @return	true if the wireframe mode is enabled, false otherwise */
		static bool hasWireframe();

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

		/** Enables or disables writing to the stencil buffer
		 *
		 * @param	active if the we should write to the stencil buffer or
		 *			not */
		static void setStencilMask(bool active);

		/** @return	true if writting to the depth buffer is enabled or not */
		static bool hasDepthMask();

		/** Adds a memory barrier for syncing image reads and writes */
		static void imageMemoryBarrier();
	};


	/**
	 * Class BindableOperation, it's a Bindable used for interacting with the
	 * Graphics API on bind or unbind
	 */
	class BindableOperation : public Bindable
	{
	public:		// Nested types
		using Callback = std::function<void()>;

	private:	// Attributes
		/** The function to call on bind */
		Callback mBindCallback;

		/** The function to call on unbind */
		Callback mUnbindCallback;

	public:		// Functions
		/** Creates a new BindableOperation
		 *
		 * @param	bindCallback the function to call on bind
		 * @param	unbindCallback the function to call on unbind */
		BindableOperation(
			const Callback& bindCallback = Callback(),
			const Callback& unbindCallback = Callback()
		) : mBindCallback(bindCallback), mUnbindCallback(unbindCallback) {};

		/** Class destructor */
		virtual ~BindableOperation() = default;

		/** @copydoc Bindable::clone() */
		virtual std::unique_ptr<Bindable> clone() const override
		{ return std::make_unique<BindableOperation>(*this); };

		/** @copydoc Bindable::bind() */
		virtual void bind() const override { mBindCallback(); };

		/** @copydoc Bindable::unbind() */
		virtual void unbind() const override { mUnbindCallback(); };
	};


	/**
	 * Class EnableOperation, it's a BindableOperation used for enabling or
	 * disabling a Graphics Operation
	 */
	class EnableOperation : public BindableOperation
	{
	private:	// Attributes
		/** If we want to enable or disable the operation on bind */
		bool mActive;

		/** The previous state of the operation */
		mutable bool mLastActive;

	public:		// Functions
		/** Creates a new EnableOperation
		 *
		 * @param	active if we want to write to enable or disable the
		 *			operation */
		EnableOperation(bool active = true);

		/** Class destructor */
		virtual ~EnableOperation() = default;

		/** @return	true if the operation will be enabled on bind, false
		 *			otherwise */
		bool getEnable() const { return mActive; };

		/** Enables/Disables the operation on bind
		 *
		 * @param	active if we want to enable the operation on bind or not */
		void setEnable(bool active = true) { mActive = active; };
	protected:
		/** The operation to call on bind
		 *
		 * @param	enable if the Operation should be enabled or not */
		virtual void enable(bool enable) const = 0;

		/** @return	true if the operation is currently enabled, false
		 *			otherwise */
		virtual bool isEnabled() const = 0;
	};


	/**
	 * Class SetOperation, it's a EnableOperation used for enabling or
	 * disabling an operation
	 */
	class SetOperation : public EnableOperation
	{
	private:	// Attributes
		/** The operation to enable/disable */
		Operation mOperation;

	public:		// Functions
		/** Creates a new SetOperation
		 *
		 * @param	operation the operation to enable/disable
		 * @param	active if the operation should be enabled or not */
		SetOperation(Operation operation, bool active = true) :
			EnableOperation(active), mOperation(operation) {};

		/** @return	the Operation to enable/disable */
		Operation getOperation() const { return mOperation; };

		/** @copydoc Bindable::clone() */
		virtual std::unique_ptr<Bindable> clone() const override
		{ return std::make_unique<SetOperation>(*this); };
	protected:
		/** @copydoc EnableOperation::enable() */
		virtual void enable(bool enable) const override
		{ return GraphicsOperations::setOperation(mOperation, enable); };

		/** @copydoc EnableOperation::isEnabled() */
		virtual bool isEnabled() const override
		{ return GraphicsOperations::hasOperation(mOperation); };
	};


	/**
	 * Class SetDeptWireframeModeOperationhMask, it's a EnableOperation used
	 * for enabling or disabling the wireframe mode
	 */
	class WireframeMode : public EnableOperation
	{
	public:		// Functions
		/** Creates a new WireframeMode
		 *
		 * @param	active if we want to enable the wireframe mode on bind or
		 *			not */
		WireframeMode(bool active = true) : EnableOperation(active) {};

		/** @copydoc Bindable::clone() */
		virtual std::unique_ptr<Bindable> clone() const override
		{ return std::make_unique<WireframeMode>(*this); };
	protected:
		/** @copydoc EnableOperation::enable() */
		virtual void enable(bool enable) const override
		{ return GraphicsOperations::setWireframe(enable); };

		/** @copydoc EnableOperation::isEnabled() */
		virtual bool isEnabled() const override
		{ return GraphicsOperations::hasWireframe(); };
	};

}

#endif		// GRAPHICS_OPERATIONS_H
