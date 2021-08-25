#include <glm/gtc/matrix_transform.hpp>
#include "se/app/graphics/GaussianBlurNode.h"
#include "se/app/io/ShaderLoader.h"
#include "se/graphics/core/Texture.h"
#include "se/graphics/core/UniformVariable.h"
#include "se/graphics/core/GraphicsOperations.h"

namespace se::app {

	GaussianBlurNode::GaussianBlurNode(const std::string& name, Repository& repository, bool horizontal) :
		BindableRenderNode(name), mFrameBuffer(nullptr)
	{
		auto iColorTexBindable = addBindable();
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("input", this, iColorTexBindable) );

		mOutputTextureBindableIndex = addBindable(nullptr, false);
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("output", this, mOutputTextureBindableIndex) );
		addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("output", this, mOutputTextureBindableIndex) );

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

		auto frameBuffer = std::make_unique<graphics::FrameBuffer>();
		mFrameBuffer = frameBuffer.get();

		addBindable(std::move(frameBuffer));
		addBindable(mProgram.get());
		addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", mProgram.get(), glm::mat4(1.0f)));
		addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix", mProgram.get(), glm::mat4(1.0f)));
		addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", mProgram.get(), glm::mat4(1.0f)));
		addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uHorizontal", mProgram.get(), horizontal));
		addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uColor", mProgram.get(), kColorTextureUnit));
	}


	void GaussianBlurNode::setBindable(std::size_t bindableIndex, const BindableSPtr& bindable)
	{
		if (bindableIndex == mOutputTextureBindableIndex) {
			mFrameBuffer->attach(std::dynamic_pointer_cast<graphics::Texture>(bindable), graphics::FrameBufferAttachment::kColor0);
		}

		BindableRenderNode::setBindable(bindableIndex, bindable);
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
