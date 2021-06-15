#include "se/app/graphics/Tex3DClearNode.h"
#include "se/app/io/ShaderLoader.h"
#include "se/graphics/core/Texture.h"
#include "se/graphics/core/UniformVariable.h"
#include "se/graphics/core/GraphicsOperations.h"

namespace se::app {

	Tex3DClearNode::Tex3DClearNode(
		const std::string& name, Repository& repository, std::size_t maxSize
	) : BindableRenderNode(name), mMaxSize(maxSize)
	{
		mProgram = repository.findByName<graphics::Program>("programTex3DClear");
		if (!mProgram) {
			std::shared_ptr<graphics::Program> program;
			auto result = ShaderLoader::createProgram("res/shaders/vertexTex3DClear.glsl", nullptr, "res/shaders/fragmentTex3DClear.glsl", program);
			if (!result) {
				SOMBRA_ERROR_LOG << result.description();
				return;
			}
			mProgram = repository.insert(std::move(program), "programTex3DClear");
		}
		addBindable(mProgram.get());

		addBindable( std::make_shared<graphics::UniformVariableValue<int>>("uMaxSize", mProgram.get(), static_cast<int>(mMaxSize)) );
		addBindable( std::make_shared<graphics::UniformVariableValue<int>>("uImage3D", mProgram.get(), kImageUnit) );

		auto tex3DIndex = addBindable();
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("input", this, tex3DIndex) );
		addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("output", this, tex3DIndex) );

		mPlane = repository.findByName<graphics::Mesh>("plane");
	}


	void Tex3DClearNode::execute()
	{
		int originX, originY;
		std::size_t dimensionsX, dimensionsY;
		graphics::GraphicsOperations::getViewport(originX, originY, dimensionsX, dimensionsY);
		graphics::GraphicsOperations::setViewport(0, 0, mMaxSize, mMaxSize);
		graphics::GraphicsOperations::setColorMask(false, false, false, false);
		graphics::SetOperation opCulling(graphics::Operation::Culling, false);		opCulling.bind();
		graphics::SetOperation opDepthTest(graphics::Operation::DepthTest, false);	opDepthTest.bind();
		graphics::SetOperation opBlending(graphics::Operation::Blending, false);	opBlending.bind();

		bind();
		mPlane->bind();
		graphics::GraphicsOperations::drawIndexedInstanced(
			graphics::PrimitiveType::Triangle,
			mPlane->getIBO().getIndexCount(), mPlane->getIBO().getIndexType(), 0,
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
