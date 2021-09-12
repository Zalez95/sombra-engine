#include "se/graphics/FBClearNode.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/graphics/core/FrameBuffer.h"

namespace se::graphics {

	FBClearNode::FBClearNode(const std::string& name, const FrameBufferMask::Mask& mask) :
		BindableRenderNode(name), mMask(mask)
	{
		auto bindableIndex = addBindable();

		addInput( std::make_unique<BindableRNodeInput<FrameBuffer>>("target", this, bindableIndex) );
		addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("target", this, bindableIndex) );
	}


	void FBClearNode::execute()
	{
		bind();
		GraphicsOperations::clear(mMask);
	}

}
