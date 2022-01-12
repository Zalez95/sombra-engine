#include "se/app/io/ShaderLoader.h"
#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/app/graphics/TypeRefs.h"
#include "GaussianBlurNode.h"

namespace se::app {

	GaussianBlurNode::GaussianBlurNode(const std::string& name, graphics::Context& context, bool horizontal) :
		BindableRenderNode(name)
	{
		auto iColorTexBindable = addBindable();
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("input", this, iColorTexBindable) );

		std::size_t targetFBBindableIndex = addBindable();
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::FrameBuffer>>("target", this, targetFBBindableIndex) );
		addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::FrameBuffer>>("target", this, targetFBBindableIndex) );

		mPlaneIndex = addBindable();
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Mesh>>("plane", this, mPlaneIndex) );

		ProgramRef program;
		auto result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentGaussianBlur.glsl", context, program);
		if (!result) {
			SOMBRA_ERROR_LOG << result.description();
			return;
		}

		addBindable(program);
		addBindable(
			context.create<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", glm::mat4(1.0f))
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		addBindable(
			context.create<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix", glm::mat4(1.0f))
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		addBindable(
			context.create<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", glm::mat4(1.0f))
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		addBindable(
			context.create<graphics::UniformVariableValue<int>>("uHorizontal", horizontal)
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		addBindable(
			context.create<graphics::UniformVariableValue<int>>("uColor", kColorTextureUnit)
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
	}


	void GaussianBlurNode::execute(graphics::Context::Query& q)
	{
		auto plane = q.getTBindable( MeshRef::from(getBindable(mPlaneIndex)) );

		bind(q);
		graphics::GraphicsOperations::drawIndexedInstanced(
			graphics::PrimitiveType::Triangle,
			plane->getIBO()->getIndexCount(), plane->getIBO()->getIndexType()
		);
	}

}
