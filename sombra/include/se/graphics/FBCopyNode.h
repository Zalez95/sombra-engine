#ifndef FB_COPY_NODE_H
#define FB_COPY_NODE_H

#include <bitset>
#include "core/Constants.h"
#include "BindableRenderNode.h"

namespace se::graphics {

	/**
	 * Class FBCopyNode, it's RenderNode that can copy the contents of one
	 * FrameBuffer to other one. It has an "input1" BindableRNodeInput where
	 * the FrameBuffer to copy into can be inserted and a "input2" where the
	 * FrameBuffer to copy from can be inserted inserted. It also has an
	 * "output" BindableRNodeOutput where the "input1" FrameBuffer can be
	 * recovered
	 */
	class FBCopyNode : public BindableRenderNode
	{
	private:	// Attributes
		/** The bit mask with the FrameBuffer buffers to copy */
		FrameBufferMask::Mask mMask;

		/** The bindable index of the FrameBuffer to copy into */
		std::size_t mFrameBuffer1Index;

		/** The bindable index of the FrameBuffer to copy from */
		std::size_t mFrameBuffer2Index;

		/** The X position of the first FrameBuffer to start copying into */
		std::size_t mX0;

		/** The Y position of the first FrameBuffer to start copying into */
		std::size_t mY0;

		/** The width to copy into the first FrameBuffer */
		std::size_t mW0;

		/** The height to copy into the first FrameBuffer */
		std::size_t mH0;

		/** The X position of the second FrameBuffer to start copying from */
		std::size_t mX1;

		/** The Y position of the second FrameBuffer to start copying from */
		std::size_t mY1;

		/** The width to copy from the second FrameBuffer */
		std::size_t mW1;

		/** The height to copy from the second FrameBuffer */
		std::size_t mH1;

		/** The filtering method to apply, Nearest or Linear */
		TextureFilter mFilter;

	public:		// Functions
		/** Creates a new FBCopyNode
		 *
		 * @param	name the name of the new FBCopyNode
		 * @param	mask a bit mask with the FrameBuffer buffers to copy */
		FBCopyNode(const std::string& name, const FrameBufferMask::Mask& mask);

		/** Sets the dimensions of the rectangle to copy into of the first
		 * FrameBuffer
		 *
		 * @param	x the X position of the first FrameBuffer to start
		 *			copying into
		 * @param	y the Y position of the first FrameBuffer to start
		 *			copying into
		 * @param	w the width to copy into the first FrameBuffer
		 * @param	h the height to copy into the first FrameBuffer
		 * @return	a reference to the current FBCopyNode */
		FBCopyNode& setDimensions1(
			std::size_t x, std::size_t y, std::size_t w, std::size_t h
		);

		/** Sets the dimensions of the rectangle to read from the second
		 * FrameBuffer
		 *
		 * @param	x the X position of the second FrameBuffer to start
		 *			copying from
		 * @param	y the Y position of the second FrameBuffer to start
		 *			copying from
		 * @param	w the width to copy from the second FrameBuffer
		 * @param	h the height to copy from the second FrameBuffer
		 * @return	a reference to the current FBCopyNode */
		FBCopyNode& setDimensions2(
			std::size_t x, std::size_t y, std::size_t w, std::size_t h
		);

		/** Sets the filtering method
		 *
		 * @param	filter the new filtering method, Nearest or Linear
		 * @return	a reference to the current FBCopyNode */
		FBCopyNode& setFilter(TextureFilter filter);

		/** Class destructor */
		virtual ~FBCopyNode() = default;

		/** @copydoc RenderNode::execute(Context::Query&) */
		virtual void execute(Context::Query& q) override;
	};

}

#endif		// FB_COPY_NODE_H
