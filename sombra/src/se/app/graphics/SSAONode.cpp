#include <glm/gtc/random.hpp>
#include "se/utils/MathUtils.h"
#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/app/graphics/SSAONode.h"
#include "se/app/io/ShaderLoader.h"

namespace se::app {

	SSAONode::SSAONode(const std::string& name, graphics::Context& context) : BindableRenderNode(name)
	{
		std::size_t ssaoBufferBindableIndex = addBindable();
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::FrameBuffer>>("target", this, ssaoBufferBindableIndex) );
		addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::FrameBuffer>>("target", this, ssaoBufferBindableIndex) );

		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("position", this, addBindable()) );
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("normal", this, addBindable()) );

		mPlaneIndex = addBindable();
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Mesh>>("plane", this, mPlaneIndex) );

		std::vector<glm::vec3> noise;
		noise.reserve(16);
		addRotationNoise(noise, 16);

		std::vector<glm::vec3> samples;
		samples.reserve(64);
		addHemisphereSamples(samples, 64);

		ProgramRef program;
		auto result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentSSAO.glsl", context, program);
		if (!result) {
			SOMBRA_ERROR_LOG << result.description();
			return;
		}
		addBindable(program);

		TextureRef rotationNoiseTexture;
		rotationNoiseTexture = context.create<graphics::Texture>(graphics::TextureTarget::Texture2D);
		rotationNoiseTexture.edit([=](graphics::Texture& texture) {
			texture.setImage(noise.data(), graphics::TypeId::Float, graphics::ColorFormat::RGB, graphics::ColorFormat::RGB16f, 4, 4)
				.setWrapping(graphics::TextureWrap::Repeat, graphics::TextureWrap::Repeat)
				.setFiltering(graphics::TextureFilter::Nearest, graphics::TextureFilter::Nearest)
				.setTextureUnit(TexUnits::kRotationNoise);
		});
		addBindable(rotationNoiseTexture);

		mViewMatrixIndex = addBindable(
			context.create<graphics::UniformVariableValue<glm::mat4>>("uFragmentViewMatrix", glm::mat4(1.0f))
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		mProjectionMatrixIndex = addBindable(
			context.create<graphics::UniformVariableValue<glm::mat4>>("uFragmentProjectionMatrix", glm::mat4(1.0f))
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		addBindable(
			context.create<graphics::UniformVariableValueVector<glm::vec3>>("uHemisphereSamples", samples)
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		addBindable(
			context.create<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", glm::mat4(1.0f))
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		addBindable(
			context.create<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix", glm::mat4(1.0f))
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		addBindable(
			context.create<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", glm::mat4(1.0f))
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		addBindable(
			context.create<graphics::UniformVariableValue<int>>("uPosition", TexUnits::kPosition)
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		addBindable(
			context.create<graphics::UniformVariableValue<int>>("uNormal", TexUnits::kNormal)
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
		addBindable(
			context.create<graphics::UniformVariableValue<int>>("uRotationNoise", TexUnits::kRotationNoise)
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
		);
	}


	void SSAONode::setViewMatrix(const glm::mat4& viewMatrix)
	{
		UniformVVRef<glm::mat4>::from( getBindable(mViewMatrixIndex) ).edit([=](auto& uniform) {
			uniform.setValue(viewMatrix);
		});
	}


	void SSAONode::setProjectionMatrix(const glm::mat4& projectionMatrix)
	{
		UniformVVRef<glm::mat4>::from( getBindable(mProjectionMatrixIndex) ).edit([=](auto& uniform) {
			uniform.setValue(projectionMatrix);
		});
	}


	void SSAONode::execute(graphics::Context::Query& q)
	{
		auto plane = q.getTBindable( MeshRef::from(getBindable(mPlaneIndex)) );

		bind(q);
		graphics::GraphicsOperations::drawIndexed(
			graphics::PrimitiveType::Triangle,
			plane->getIBO()->getIndexCount(), plane->getIBO()->getIndexType()
		);
	}

// Private functions
	void SSAONode::addHemisphereSamples(std::vector<glm::vec3>& kernel, std::size_t numSamples)
	{
		for (std::size_t i = 0; i < numSamples; ++i) {
			// Generate a random direction in a hemisphere
			glm::vec3 sample(glm::linearRand(-1.0f, 1.0f), glm::linearRand(-1.0f, 1.0f), glm::linearRand(0.0f, 1.0f));
			sample = glm::normalize(sample);

			// Calculate the scale value with a accelerating interpolation function
			float scale = i / float(numSamples);
			scale = utils::lerp(0.1f, 1.0f, scale * scale);

			// Scale the direction
			sample *= scale;
			kernel.push_back(sample);
		}
	}


	void SSAONode::addRotationNoise(std::vector<glm::vec3>& kernel, std::size_t numVectors)
	{
		for (std::size_t i = 0; i < numVectors; ++i) {
			glm::vec3 sample(glm::linearRand(-1.0f, 1.0f), glm::linearRand(-1.0f, 1.0f), 0.0f);
			kernel.push_back(sample);
		}
	}

}
