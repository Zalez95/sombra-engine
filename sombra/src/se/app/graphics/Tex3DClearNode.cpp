#include "se/app/graphics/Tex3DClearNode.h"
#include "se/app/loaders/TechniqueLoader.h"
#include "se/graphics/core/Texture.h"
#include "se/graphics/core/UniformVariable.h"
#include "se/graphics/core/GraphicsOperations.h"

namespace se::app {

	Tex3DClearNode::Tex3DClearNode(
		const std::string& name, utils::Repository& repository, std::size_t maxSize
	) : BindableRenderNode(name), mMaxSize(maxSize)
	{
		auto program = repository.find<std::string, graphics::Program>("programTex3DClear");
		if (!program) {
			program = TechniqueLoader::createProgram(
				"res/shaders/vertexTex3DClear.glsl", nullptr, "res/shaders/fragmentTex3DClear.glsl"
			);
			repository.add(std::string("programTex3DClear"), program);
		}
		addBindable(program);

		addBindable( std::make_shared<graphics::UniformVariableValue<int>>("uMaxSize", *program, static_cast<int>(mMaxSize)) );
		addBindable( std::make_shared<graphics::UniformVariableValue<int>>("uImage3D", *program, kImageUnit) );

		auto tex3DIndex = addBindable();
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("input", this, tex3DIndex) );
		addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("output", this, tex3DIndex) );

		// Create the plane
		graphics::VertexArray vao;
		vao.bind();

		std::vector<graphics::VertexBuffer> vbos;
		auto& vbo = vbos.emplace_back();
		glm::vec2 positions[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
		vbo.resizeAndCopy(positions, 4);
		vbo.bind();
		vao.setVertexAttribute(0, graphics::TypeId::Float, false, 2, 0);

		graphics::IndexBuffer ibo;
		unsigned short faceIndices[] = { 0, 1, 2, 2, 3, 0 };
		ibo.resizeAndCopy(faceIndices, graphics::TypeId::UnsignedShort, 6);
		ibo.bind();

		mPlane = std::make_unique<graphics::Mesh>(std::move(vbos), std::move(ibo), std::move(vao));
	}


	void Tex3DClearNode::execute()
	{
		int originX, originY;
		std::size_t dimensionsX, dimensionsY;
		graphics::GraphicsOperations::getViewport(originX, originY, dimensionsX, dimensionsY);
		graphics::GraphicsOperations::setViewport(0, 0, mMaxSize, mMaxSize);
		graphics::GraphicsOperations::setColorMask(false, false, false, false);

		graphics::GraphicsOperations::setCulling(false);
		graphics::GraphicsOperations::setDepthTest(false);
		graphics::GraphicsOperations::setBlending(false);

		bind();
		mPlane->bind();
		graphics::GraphicsOperations::drawIndexedInstanced(
			graphics::PrimitiveType::Triangle,
			mPlane->getIBO().getIndexCount(), mPlane->getIBO().getIndexType(),
			mMaxSize
		);
		graphics::GraphicsOperations::imageMemoryBarrier();

		graphics::GraphicsOperations::setColorMask(true, true, true, true);
		graphics::GraphicsOperations::setViewport(originX, originY, dimensionsX, dimensionsY);
	}

}
