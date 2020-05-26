#ifndef FB_CLEAR_NODE_H
#define FB_CLEAR_NODE_H

#include "BindableRenderNode.h"

namespace se::graphics {

	/**
	 * Class FBClearNode, it's RenderNode that can clean FrameBuffers. It has
	 * an "input" BindableRNodeInput where the FrameBuffer to clean can be
	 * inserted and an "output" BindableRNodeOutput where the same FrameBuffer
	 * can be recovered
	 */
	class FBClearNode : public BindableRenderNode
	{
	public:		// Functions
		/** Creates a new FBClearNode
		 *
		 * @param	name the name of the new FBClearNode */
		FBClearNode(const std::string& name);

		/** Executes the current RenderNode */
		virtual void execute() override;
	};

}

#endif		// FB_CLEAR_NODE_H
