#include "se/graphics/FBClearNode.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/graphics/core/FrameBuffer.h"

namespace se::graphics {

	FBClearNode::FBClearNode(const std::string& name, bool color, bool depth, bool stencil) :
		BindableRenderNode(name), mColor(color), mDepth(depth), mStencil(stencil)
	{
		auto bindableIndex = addBindable();

		addInput( std::make_unique<BindableRNodeInput<FrameBuffer>>("input", this, bindableIndex) );
		addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("output", this, bindableIndex) );
	}


	void FBClearNode::execute()
	{
		bind();
		GraphicsOperations::clear(mColor, mDepth, mStencil);
	}

}
