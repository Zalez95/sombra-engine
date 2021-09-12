#include "se/app/graphics/FXAANode.h"
#include "se/app/io/ShaderLoader.h"
#include "se/graphics/core/Texture.h"
#include "se/graphics/core/UniformVariable.h"
#include "se/graphics/core/GraphicsOperations.h"

namespace se::app {

	FXAANode::FXAANode(const std::string& name, Repository& repository) :
		BindableRenderNode(name)
	{
		auto iColorTexBindable = addBindable();
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("input", this, iColorTexBindable) );

		std::size_t targetFBBindableIndex = addBindable();
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::FrameBuffer>>("target", this, targetFBBindableIndex) );
		addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::FrameBuffer>>("target", this, targetFBBindableIndex) );

		mProgram = repository.findByName<graphics::Program>("programFXAA");
		if (!mProgram) {
			std::shared_ptr<graphics::Program> program;
			auto result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentFXAA.glsl", program);
			if (!result) {
				SOMBRA_ERROR_LOG << result.description();
				return;
			}
			mProgram = repository.insert(std::move(program), "programFXAA");
		}

		mPlane = repository.findByName<graphics::Mesh>("plane");
		if (!mPlane) {
			SOMBRA_ERROR_LOG << "plane not found";
			return;
		}

		addBindable(mProgram.get());
		addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", mProgram.get(), glm::mat4(1.0f)));
		addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix", mProgram.get(), glm::mat4(1.0f)));
		addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", mProgram.get(), glm::mat4(1.0f)));
		addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uColor", mProgram.get(), kColorTextureUnit));
	}


	void FXAANode::execute()
	{
		bind();
		mPlane->bind();
		graphics::GraphicsOperations::drawIndexedInstanced(
			graphics::PrimitiveType::Triangle,
			mPlane->getIBO().getIndexCount(), mPlane->getIBO().getIndexType()
		);
	}

}
