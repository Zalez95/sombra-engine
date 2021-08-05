#include "se/graphics/core/Texture.h"
#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/app/graphics/DeferredAmbientRenderer.h"
#include "se/app/io/ShaderLoader.h"
#include "se/app/io/MeshLoader.h"

namespace se::app {

	DeferredAmbientRenderer::DeferredAmbientRenderer(const std::string& name, Repository& repository) :
		BindableRenderNode(name)
	{
		auto iTargetBindable = addBindable();
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::FrameBuffer>>("target", this, iTargetBindable) );
		addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::FrameBuffer>>("target", this, iTargetBindable) );

		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("irradiance", this, addBindable()) );
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("prefilter", this, addBindable()) );
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("brdf", this, addBindable()) );
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("position", this, addBindable()) );
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("normal", this, addBindable()) );
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("albedo", this, addBindable()) );
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("material", this, addBindable()) );
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("emissive", this, addBindable()) );

		mProgram = repository.findByName<graphics::Program>("programDeferredAmbient");
		if (!mProgram) {
			std::shared_ptr<graphics::Program> program;
			auto result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentDeferredAmbient.glsl", program);
			if (!result) {
				SOMBRA_ERROR_LOG << result.description();
				return;
			}
			mProgram = repository.insert(std::move(program), "programDeferredAmbient");
		}

		mPlane = repository.findByName<graphics::Mesh>("plane");
		if (!mPlane) {
			SOMBRA_ERROR_LOG << "plane not found";
			return;
		}

		mViewPosition = std::make_shared<graphics::UniformVariableValue<glm::vec3>>("uViewPosition", mProgram.get(), glm::vec3(0.0f));

		addBindable(mProgram.get());
		addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", mProgram.get(), glm::mat4(1.0f)));
		addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix", mProgram.get(), glm::mat4(1.0f)));
		addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", mProgram.get(), glm::mat4(1.0f)));
		addBindable(mViewPosition);
		addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uIrradianceMap", mProgram.get(), TexUnits::kIrradianceMap));
		addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uPrefilterMap", mProgram.get(), TexUnits::kPrefilterMap));
		addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uBRDFMap", mProgram.get(), TexUnits::kBRDFMap));
		addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uPosition", mProgram.get(), TexUnits::kPosition));
		addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uNormal", mProgram.get(), TexUnits::kNormal));
		addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uAlbedo", mProgram.get(), TexUnits::kAlbedo));
		addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uMaterial", mProgram.get(), TexUnits::kMaterial));
		addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uEmissive", mProgram.get(), TexUnits::kEmissive));
	}


	void DeferredAmbientRenderer::setViewPosition(const glm::vec3& position)
	{
		mViewPosition->setValue(position);
	}


	void DeferredAmbientRenderer::execute()
	{
		graphics::GraphicsOperations::setDepthMask(false);

		bind();
		mPlane->bind();
		graphics::GraphicsOperations::drawIndexed(
			graphics::PrimitiveType::Triangle,
			mPlane->getIBO().getIndexCount(), mPlane->getIBO().getIndexType()
		);

		graphics::GraphicsOperations::setDepthMask(true);
	}

}
