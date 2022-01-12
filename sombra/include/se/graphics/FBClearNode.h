#ifndef FB_CLEAR_NODE_H
#define FB_CLEAR_NODE_H

#include <bitset>
#include "core/Constants.h"
#include "BindableRenderNode.h"

namespace se::graphics {

	/**
	 * Class FBClearNode, it's RenderNode that can clean FrameBuffers. It has
	 * an "target" BindableRNodeInput where the FrameBuffer to clean can be
	 * inserted and an "target" BindableRNodeOutput where the same FrameBuffer
	 * can be recovered
	 */
	class FBClearNode : public BindableRenderNode
	{
	private:	// Attributes
		/** The bit mask with the FrameBuffer buffers to clear */
		FrameBufferMask::Mask mMask;

	public:		// Functions
		/** Creates a new FBClearNode
		 *
		 * @param	name the name of the new FBClearNode
		 * @param	mask a bit mask with the FrameBuffer buffers to clear */
		FBClearNode(const std::string& name, const FrameBufferMask::Mask& mask);

		/** Class destructor */
		virtual ~FBClearNode() = default;

		/** @copydoc RenderNode::execute(Context::Query&) */
		virtual void execute(Context::Query& q) override;
	};

}

#endif		// FB_CLEAR_NODE_H
