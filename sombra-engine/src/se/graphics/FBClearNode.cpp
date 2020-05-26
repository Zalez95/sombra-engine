#include "se/graphics/FBClearNode.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/graphics/core/FrameBuffer.h"

namespace se::graphics {

	FBClearNode::FBClearNode(const std::string& name) : BindableRenderNode(name)
	{
		auto bindableIndex = addBindable();

		addInput( std::make_unique<BindableRNodeInput<FrameBuffer>>("input", this, bindableIndex) );
		addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("output", this, bindableIndex) );
	}


	void FBClearNode::execute()
	{
		bind();
		GraphicsOperations::clear(false, true);
	}

}
