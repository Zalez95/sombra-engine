#include "se/graphics/core/Texture.h"
#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/UniformBlock.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/app/graphics/DeferredLightRenderer.h"
#include "se/app/io/ShaderLoader.h"

namespace se::app {

	DeferredLightRenderer::DeferredLightRenderer(const std::string& name, Repository& repository) :
		BindableRenderNode(name)
	{
		using namespace graphics;

		auto iTargetBindable = addBindable();
		addInput( std::make_unique<BindableRNodeInput<FrameBuffer>>("target", this, iTargetBindable) );
		addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("target", this, iTargetBindable) );

		addInput( std::make_unique<BindableRNodeInput<Texture>>("irradiance", this, addBindable()) );
		addInput( std::make_unique<BindableRNodeInput<Texture>>("prefilter", this, addBindable()) );
		addInput( std::make_unique<BindableRNodeInput<Texture>>("brdf", this, addBindable()) );
		addInput( std::make_unique<BindableRNodeInput<Texture>>("shadow", this, addBindable()) );
		addInput( std::make_unique<BindableRNodeInput<Texture>>("position", this, addBindable()) );
		addInput( std::make_unique<BindableRNodeInput<Texture>>("normal", this, addBindable()) );
		addInput( std::make_unique<BindableRNodeInput<Texture>>("albedo", this, addBindable()) );
		addInput( std::make_unique<BindableRNodeInput<Texture>>("material", this, addBindable()) );
		addInput( std::make_unique<BindableRNodeInput<Texture>>("emissive", this, addBindable()) );

		mProgram = repository.findByName<Program>("programDeferredLighting");
		if (!mProgram) {
			std::shared_ptr<Program> program;
			auto result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentDeferredLighting.glsl", program);
			if (!result) {
				SOMBRA_ERROR_LOG << result.description();
				return;
			}
			mProgram = repository.insert(std::move(program), "programDeferredLighting");
		}

		mPlane = repository.findByName<Mesh>("plane");
		if (!mPlane) {
			SOMBRA_ERROR_LOG << "plane not found";
			return;
		}

		mViewPosition = std::make_shared<UniformVariableValue<glm::vec3>>("uViewPosition", mProgram.get(), glm::vec3(0.0f));
		mNumLights = std::make_shared<UniformVariableValue<unsigned int>>("uNumLights", mProgram.get(), 0);
		mLightsBuffer = std::make_shared<UniformBuffer>();
		utils::FixedVector<ShaderLightSource, kMaxLights> lightsBufferData(kMaxLights);
		mLightsBuffer->resizeAndCopy(lightsBufferData.data(), lightsBufferData.size());

		addBindable(mProgram.get());
		addBindable(std::make_shared<SetDepthMask>(false));
		addBindable(std::make_shared<UniformVariableValue<glm::mat4>>("uModelMatrix", mProgram.get(), glm::mat4(1.0f)));
		addBindable(std::make_shared<UniformVariableValue<glm::mat4>>("uViewMatrix", mProgram.get(), glm::mat4(1.0f)));
		addBindable(std::make_shared<UniformVariableValue<glm::mat4>>("uProjectionMatrix", mProgram.get(), glm::mat4(1.0f)));
		addBindable(mViewPosition);
		addBindable(std::make_shared<UniformVariableValue<int>>("uIrradianceMap", mProgram.get(), TexUnits::kIrradianceMap));
		addBindable(std::make_shared<UniformVariableValue<int>>("uPrefilterMap", mProgram.get(), TexUnits::kPrefilterMap));
		addBindable(std::make_shared<UniformVariableValue<int>>("uBRDFMap", mProgram.get(), TexUnits::kBRDFMap));
		addBindable(std::make_shared<UniformVariableValue<int>>("uShadows", mProgram.get(), TexUnits::kShadows));
		addBindable(std::make_shared<UniformVariableValue<int>>("uPosition", mProgram.get(), TexUnits::kPosition));
		addBindable(std::make_shared<UniformVariableValue<int>>("uNormal", mProgram.get(), TexUnits::kNormal));
		addBindable(std::make_shared<UniformVariableValue<int>>("uAlbedo", mProgram.get(), TexUnits::kAlbedo));
		addBindable(std::make_shared<UniformVariableValue<int>>("uMaterial", mProgram.get(), TexUnits::kMaterial));
		addBindable(std::make_shared<UniformVariableValue<int>>("uEmissive", mProgram.get(), TexUnits::kEmissive));
		addBindable(mLightsBuffer);
		addBindable(mNumLights);
		addBindable(std::make_shared<UniformBlock>("LightsBlock", mProgram.get()));
	}


	void DeferredLightRenderer::setViewPosition(const glm::vec3& position)
	{
		mViewPosition->setValue(position);
	}


	void DeferredLightRenderer::setLights(const ShaderLightSource* lightSources, unsigned int lightSourceCount)
	{
		mLightsBuffer->copy(lightSources, lightSourceCount);
		mNumLights->setValue(lightSourceCount);
	}


	void DeferredLightRenderer::execute()
	{
		bind();
		mPlane->bind();
		graphics::GraphicsOperations::drawIndexed(
			graphics::PrimitiveType::Triangle,
			mPlane->getIBO().getIndexCount(), mPlane->getIBO().getIndexType()
		);
	}

}
