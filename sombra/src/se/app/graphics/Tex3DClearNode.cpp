#include "se/app/graphics/Tex3DClearNode.h"
#include "se/app/io/ShaderLoader.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/app/graphics/TypeRefs.h"

namespace se::app {

	Tex3DClearNode::Tex3DClearNode(const std::string& name, graphics::Context& context, std::size_t maxSize) :
		BindableRenderNode(name), mMaxSize(maxSize)
	{
		ProgramRef program;
		auto result = ShaderLoader::createProgram("res/shaders/vertexTex3DClear.glsl", nullptr, "res/shaders/fragmentTex3DClear.glsl", context, program);
		if (!result) {
			SOMBRA_ERROR_LOG << result.description();
			return;
		}

		addBindable(program);
		addBindable(
			context.create<graphics::UniformVariableValue<int>>("uMaxSize", static_cast<int>(mMaxSize))
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		addBindable(
			context.create<graphics::UniformVariableValue<int>>("uImage3D", kImageUnit)
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);

		auto tex3DIndex = addBindable();
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("input", this, tex3DIndex) );
		addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("output", this, tex3DIndex) );

		mPlaneIndex = addBindable();
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Mesh>>("plane", this, mPlaneIndex) );
	}


	void Tex3DClearNode::execute(graphics::Context::Query& q)
	{
		auto plane = q.getTBindable( MeshRef::from(getBindable(mPlaneIndex)) );

		int originX, originY;
		std::size_t dimensionsX, dimensionsY;
		graphics::GraphicsOperations::getViewport(originX, originY, dimensionsX, dimensionsY);
		graphics::GraphicsOperations::setViewport(0, 0, mMaxSize, mMaxSize);
		graphics::GraphicsOperations::setColorMask(false, false, false, false);
		graphics::SetOperation opCulling(graphics::Operation::Culling, false);		opCulling.bind();
		graphics::SetOperation opDepthTest(graphics::Operation::DepthTest, false);	opDepthTest.bind();
		graphics::SetOperation opBlending(graphics::Operation::Blending, false);	opBlending.bind();

		bind(q);
		graphics::GraphicsOperations::drawIndexedInstanced(
			graphics::PrimitiveType::Triangle,
			plane->getIBO()->getIndexCount(), plane->getIBO()->getIndexType(), 0,
			mMaxSize
		);
		graphics::GraphicsOperations::imageMemoryBarrier();

		opBlending.unbind();
		opDepthTest.unbind();
		opCulling.unbind();
		graphics::GraphicsOperations::setColorMask(true, true, true, true);
		graphics::GraphicsOperations::setViewport(originX, originY, dimensionsX, dimensionsY);
	}

}
