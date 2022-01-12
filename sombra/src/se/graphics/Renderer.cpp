#include "se/graphics/Renderer.h"
#include "se/graphics/core/FrameBuffer.h"

namespace se::graphics {

	Renderer::Renderer(const std::string& name) : BindableRenderNode(name)
	{
		auto targetIndex = addBindable();
		addInput(std::make_unique<graphics::BindableRNodeInput<graphics::FrameBuffer>>("target", this, targetIndex));
		addOutput(std::make_unique<graphics::BindableRNodeOutput<graphics::FrameBuffer>>("target", this, targetIndex));
	}


	void Renderer::execute(Context::Query& q)
	{
		sortQueue();
		bind(q);
		render(q);
		clearQueue();
	}

}
