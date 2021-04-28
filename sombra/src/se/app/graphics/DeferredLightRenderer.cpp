#include "se/graphics/core/Texture.h"
#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/UniformBlock.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/app/graphics/DeferredLightRenderer.h"
#include "se/app/io/ShaderLoader.h"

namespace se::app {

	DeferredLightRenderer::DeferredLightRenderer(const std::string& name, utils::Repository& repository) :
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

		mPlane = std::make_shared<RenderableMesh>( repository.find<std::string, Mesh>("plane") );

		auto program = repository.find<std::string, Program>("programDeferredLighting");
		if (!program) {
			auto result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentDeferredLighting.glsl", program);
			if (!result) {
				SOMBRA_ERROR_LOG << result.description();
				return;
			}
			repository.add(std::string("programDeferredLighting"), program);
		}

		mViewPosition = std::make_shared<UniformVariableValue<glm::vec3>>("uViewPosition", program, glm::vec3(0.0f));
		mNumLights = std::make_shared<UniformVariableValue<unsigned int>>("uNumLights", program, 0);
		mLightsBuffer = std::make_shared<UniformBuffer>();
		utils::FixedVector<ShaderLightSource, kMaxLights> lightsBufferData(kMaxLights);
		mLightsBuffer->resizeAndCopy(lightsBufferData.data(), lightsBufferData.size());
		mShadowLightIndex = std::make_shared<UniformVariableValue<unsigned int>>("uShadowLightIndex", program, kMaxLights);
		mShadowViewProjectionMatrix = std::make_shared<UniformVariableValue<glm::mat4>>("uShadowViewProjectionMatrix", program, glm::mat4(1.0f));

		addBindable(program);
		addBindable(std::make_shared<SetDepthMask>(false));
		addBindable(std::make_shared<UniformVariableValue<glm::mat4>>("uModelMatrix", program, glm::mat4(1.0f)));
		addBindable(std::make_shared<UniformVariableValue<glm::mat4>>("uViewMatrix", program, glm::mat4(1.0f)));
		addBindable(std::make_shared<UniformVariableValue<glm::mat4>>("uProjectionMatrix", program, glm::mat4(1.0f)));
		addBindable(mShadowViewProjectionMatrix);
		addBindable(mViewPosition);
		addBindable(std::make_shared<UniformVariableValue<int>>("uIrradianceMap", program, TexUnits::kIrradianceMap));
		addBindable(std::make_shared<UniformVariableValue<int>>("uPrefilterMap", program, TexUnits::kPrefilterMap));
		addBindable(std::make_shared<UniformVariableValue<int>>("uBRDFMap", program, TexUnits::kBRDFMap));
		addBindable(std::make_shared<UniformVariableValue<int>>("uShadowMap", program, TexUnits::kShadowMap));
		addBindable(std::make_shared<UniformVariableValue<int>>("uPosition", program, TexUnits::kPosition));
		addBindable(std::make_shared<UniformVariableValue<int>>("uNormal", program, TexUnits::kNormal));
		addBindable(std::make_shared<UniformVariableValue<int>>("uAlbedo", program, TexUnits::kAlbedo));
		addBindable(std::make_shared<UniformVariableValue<int>>("uMaterial", program, TexUnits::kMaterial));
		addBindable(std::make_shared<UniformVariableValue<int>>("uEmissive", program, TexUnits::kEmissive));
		addBindable(mLightsBuffer);
		addBindable(mNumLights);
		addBindable(mShadowLightIndex);
		addBindable(std::make_shared<UniformBlock>("LightsBlock", program));
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


	void DeferredLightRenderer::setShadowLightIndex(unsigned int shadowLightIndex)
	{
		mShadowLightIndex->setValue(shadowLightIndex);
	}


	void DeferredLightRenderer::setShadowViewProjectionMatrix(const glm::mat4& shadowViewProjectionMatrix)
	{
		mShadowViewProjectionMatrix->setValue(shadowViewProjectionMatrix);
	}


	void DeferredLightRenderer::execute()
	{
		bind();
		mPlane->draw();
	}

}
