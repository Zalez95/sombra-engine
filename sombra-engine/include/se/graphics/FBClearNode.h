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
	private:	// Attributes
		/** If the color buffer should be cleared or not */
		bool mColor;

		/** If the depth buffer should be cleared or not */
		bool mDepth;

		/** If the stencil buffer should be cleared or not */
		bool mStencil;

	public:		// Functions
		/** Creates a new FBClearNode
		 *
		 * @param	name the name of the new FBClearNode
		 * @param	color if the color buffer should be cleared or not
		 * @param	depth if the depth buffer should be cleared or not
		 * @param	stencil if the stencil buffer should be cleared or not */
		FBClearNode(
			const std::string& name,
			bool color = false, bool depth = false, bool stencil = false
		);

		/** Class destructor */
		virtual ~FBClearNode() = default;

		/** Executes the current RenderNode */
		virtual void execute() override;
	};

}

#endif		// FB_CLEAR_NODE_H
