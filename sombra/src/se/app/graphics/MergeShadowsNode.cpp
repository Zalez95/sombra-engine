#include "se/graphics/core/Texture.h"
#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/app/io/ShaderLoader.h"
#include "MergeShadowsNode.h"

namespace se::app {

	using namespace graphics;


	MergeShadowsNode::MergeShadowsNode(const std::string& name, Repository& repository) : graphics::BindableRenderNode(name)
	{
		// Connections
		addInput( std::make_unique<RNodeInput>("attach", this) );
		addOutput( std::make_unique<RNodeOutput>("attach", this) );

		auto iTargetBindable = addBindable();
		addInput( std::make_unique<BindableRNodeInput<FrameBuffer>>("target", this, iTargetBindable) );
		addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("target", this, iTargetBindable) );

		auto iPositionTextureBindable = addBindable();
		addInput( std::make_unique<BindableRNodeInput<Texture>>("position", this, iPositionTextureBindable) );

		auto iNormalTextureBindable = addBindable();
		addInput( std::make_unique<BindableRNodeInput<Texture>>("normal", this, iNormalTextureBindable) );

		for (std::size_t i = 0; i < kMaxShadows; ++i) {
			std::size_t iShadowTextureBindable = addBindable();
			addInput( std::make_unique<BindableRNodeInput<Texture>>("shadowTexture" + std::to_string(i), this, iShadowTextureBindable) );
		}

		// Resources
		mProgram = repository.findByName<Program>("programMergeShadows");
		if (!mProgram) {
			std::shared_ptr<Program> program;
			auto result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentMergeShadows.glsl", program);
			if (!result) {
				SOMBRA_ERROR_LOG << result.description();
				return;
			}
			mProgram = repository.insert(std::move(program), "programMergeShadows");
		}

		mPlane = repository.findByName<Mesh>("plane");
		if (!mPlane) {
			SOMBRA_ERROR_LOG << "plane not found";
			return;
		}

		mInvCameraViewProjectionMatrix = std::make_shared<UniformVariableValue<glm::mat4>>("uInvCameraViewProjectionMatrix", mProgram.get());

		addBindable(mProgram.get());
		addBindable(std::make_shared<SetDepthMask>(false));
		addBindable(std::make_shared<UniformVariableValue<glm::mat4>>("uModelMatrix", mProgram.get(), glm::mat4(1.0f)));
		addBindable(std::make_shared<UniformVariableValue<glm::mat4>>("uViewMatrix", mProgram.get(), glm::mat4(1.0f)));
		addBindable(std::make_shared<UniformVariableValue<glm::mat4>>("uProjectionMatrix", mProgram.get(), glm::mat4(1.0f)));
		addBindable(std::make_shared<UniformVariableValue<int>>("uPosition", mProgram.get(), kPositionTextureUnit));
		addBindable(std::make_shared<UniformVariableValue<int>>("uNormal", mProgram.get(), kNormalTextureUnit));
		addBindable(mInvCameraViewProjectionMatrix);
		for (std::size_t i = 0; i < kMaxShadows; ++i) {
			std::string index = std::to_string(i);

			mShadows[i].active = std::make_shared<UniformVariableValue<int>>(("uShadows[" + index + "].active").c_str(), mProgram.get(), 0);
			mShadows[i].viewProjectionMatrix = std::make_shared<UniformVariableValue<glm::mat4>>(("uShadows[" + index + "].viewProjectionMatrix").c_str(), mProgram.get());
			addBindable(mShadows[i].active);
			addBindable(mShadows[i].viewProjectionMatrix);
			addBindable(std::make_shared<UniformVariableValue<int>>(("uShadows[" + index + "].shadowMap").c_str(), mProgram.get(), i));
		}
	}


	void MergeShadowsNode::execute()
	{
		bind();
		mPlane->bind();
		GraphicsOperations::drawIndexed(
			PrimitiveType::Triangle,
			mPlane->getIBO().getIndexCount(), mPlane->getIBO().getIndexType()
		);
	}

}
