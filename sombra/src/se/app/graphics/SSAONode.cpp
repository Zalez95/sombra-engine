#include <glm/gtc/random.hpp>
#include "se/utils/MathUtils.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/app/graphics/SSAONode.h"
#include "se/app/io/ShaderLoader.h"

namespace se::app {

	SSAONode::SSAONode(const std::string& name, Repository& repository) :
		BindableRenderNode(name)
	{
		mProgram = repository.findByName<graphics::Program>("programSSAO");
		if (!mProgram) {
			std::shared_ptr<graphics::Program> program;
			auto result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentSSAO.glsl", program);
			if (!result) {
				SOMBRA_ERROR_LOG << result.description();
				return;
			}
			mProgram = repository.insert(std::move(program), "programSSAO");
		}

		mPlane = repository.findByName<graphics::Mesh>("plane");
		if (!mPlane) {
			SOMBRA_ERROR_LOG << "plane not found";
			return;
		}

		mRotationNoiseTexture = repository.findByName<graphics::Texture>("rotationNoiseTexture");
		if (!mRotationNoiseTexture) {
			std::vector<glm::vec3> noise;
			noise.reserve(16);
			addRotationNoise(noise, 16);

			auto texture = std::make_shared<graphics::Texture>(graphics::TextureTarget::Texture2D);
			texture->setImage(noise.data(), graphics::TypeId::Float, graphics::ColorFormat::RGB, graphics::ColorFormat::RGB16f, 4, 4)
				.setWrapping(graphics::TextureWrap::Repeat, graphics::TextureWrap::Repeat)
				.setFiltering(graphics::TextureFilter::Nearest, graphics::TextureFilter::Nearest)
				.setTextureUnit(TexUnits::kRotationNoise);

			mRotationNoiseTexture = repository.insert(std::move(texture), "rotationNoiseTexture");
		}

		mViewMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uFragmentViewMatrix", mProgram.get(), glm::mat4(1.0f));
		mProjectionMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uFragmentProjectionMatrix", mProgram.get(), glm::mat4(1.0f));

		std::vector<glm::vec3> samples;
		samples.reserve(64);
		addHemisphereSamples(samples, 64);
		auto hemisphereSamples = std::make_shared<graphics::UniformVariableValueVector<glm::vec3>>("uHemisphereSamples", mProgram.get(), samples.data(), samples.size());

		addBindable(mProgram.get());
		addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", mProgram.get(), glm::mat4(1.0f)));
		addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix", mProgram.get(), glm::mat4(1.0f)));
		addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", mProgram.get(), glm::mat4(1.0f)));
		addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uPosition", mProgram.get(), TexUnits::kPosition));
		addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uNormal", mProgram.get(), TexUnits::kNormal));
		addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uRotationNoise", mProgram.get(), TexUnits::kRotationNoise));
		addBindable(mRotationNoiseTexture.get());
		addBindable(mViewMatrix);
		addBindable(mProjectionMatrix);
		addBindable(hemisphereSamples);

		std::size_t ssaoBufferBindableIndex = addBindable();
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::FrameBuffer>>("target", this, ssaoBufferBindableIndex) );
		addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::FrameBuffer>>("target", this, ssaoBufferBindableIndex) );

		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("position", this, addBindable()) );
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("normal", this, addBindable()) );
	}


	void SSAONode::setViewMatrix(const glm::mat4& viewMatrix)
	{
		mViewMatrix->setValue(viewMatrix);
	}


	void SSAONode::setProjectionMatrix(const glm::mat4& projectionMatrix)
	{
		mProjectionMatrix->setValue(projectionMatrix);
	}


	void SSAONode::execute()
	{
		bind();
		mPlane->bind();
		graphics::GraphicsOperations::drawIndexed(
			graphics::PrimitiveType::Triangle,
			mPlane->getIBO().getIndexCount(), mPlane->getIBO().getIndexType()
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
