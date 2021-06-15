#include <glm/gtc/matrix_transform.hpp>
#include "se/app/graphics/GaussianBlurNode.h"
#include "se/app/io/ShaderLoader.h"
#include "se/graphics/core/Texture.h"
#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/UniformVariable.h"
#include "se/graphics/core/GraphicsOperations.h"

namespace se::app {

	GaussianBlurNode::GaussianBlurNode(
		const std::string& name, Repository& repository,
		std::size_t width, std::size_t height, bool horizontal
	) : BindableRenderNode(name)
	{
		auto iColorTexBindable = addBindable();
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("input", this, iColorTexBindable) );

		auto frameBuffer = std::make_unique<graphics::FrameBuffer>();
		auto outputTexture = std::make_unique<graphics::Texture>(graphics::TextureTarget::Texture2D);
		outputTexture->setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::RGBA, graphics::ColorFormat::RGBA16f, width, height)
			.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
			.setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear);
		frameBuffer->attach(*outputTexture, graphics::FrameBufferAttachment::kColor0);
		auto iOutputTexBindable = addBindable(std::move(outputTexture), false);
		addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("output", this, iOutputTexBindable) );

		mProgram = repository.findByName<graphics::Program>("programGaussianBlur");
		if (!mProgram) {
			std::shared_ptr<graphics::Program> program;
			auto result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentGaussianBlur.glsl", program);
			if (!result) {
				SOMBRA_ERROR_LOG << result.description();
				return;
			}
			mProgram = repository.insert(std::move(program), "programGaussianBlur");
		}

		mPlane = repository.findByName<graphics::Mesh>("plane");
		if (!mPlane) {
			SOMBRA_ERROR_LOG << "plane not found";
			return;
		}

		addBindable(std::move(frameBuffer));
		addBindable(mProgram.get());
		addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", mProgram.get(), glm::mat4(1.0f)));
		addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix", mProgram.get(), glm::mat4(1.0f)));
		addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", mProgram.get(), glm::mat4(1.0f)));
		addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uHorizontal", mProgram.get(), horizontal));
		addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uColor", mProgram.get(), kColorTextureUnit));
	}


	void GaussianBlurNode::setTextureDimensions(std::size_t width, std::size_t height)
	{
		auto output = dynamic_cast<graphics::BindableRNodeOutput<graphics::Texture>*>(findOutput("output"));
		auto texture = std::dynamic_pointer_cast<graphics::Texture>(output->getBindable());
		texture->setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::RGBA, graphics::ColorFormat::RGBA16f, width, height);
	}


	void GaussianBlurNode::execute()
	{
		graphics::GraphicsOperations::setDepthMask(false);

		bind();

		auto mask = graphics::FrameBufferMask::Mask().set(graphics::FrameBufferMask::kColor);
		graphics::GraphicsOperations::clear(mask);

		mPlane->bind();
		graphics::GraphicsOperations::drawIndexedInstanced(
			graphics::PrimitiveType::Triangle,
			mPlane->getIBO().getIndexCount(), mPlane->getIBO().getIndexType()
		);

		graphics::GraphicsOperations::setDepthMask(true);
	}

}
