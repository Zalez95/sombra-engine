#include <cmath>
#include <limits>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>
#include "se/app/ParticleSystemComponent.h"
#include "se/app/events/RenderableShaderEvent.h"

#define RANDOM_ZERO_ONE() (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX))

namespace se::app {

	ParticleSystemComponent::ParticleSystemComponent(
		EventManager& eventManager, Entity entity, graphics::PrimitiveType primitiveType
	) : mEventManager(eventManager), mEntity(entity), mParticleSystem(nullptr, primitiveType),
		mInitialPosition(0.0f), mInitialOrientation(1.0f, glm::vec3(0.0f)), mAccumulatedTime(0.0f)
	{
		auto& vao = mParticleSystem.getVAO();
		auto& vbo = mParticleSystem.getInstanceVBOs().emplace_back();

		vao.bind();
		vbo.bind();
		vao.enableAttribute(kPositionIndex);
		vao.setVertexAttribute(kPositionIndex, graphics::TypeId::Float, false, 3, 36, offsetof(Particle, position));
		vao.setAttributeDivisor(kPositionIndex, 1);
		vao.enableAttribute(kVelocityIndex);
		vao.setVertexAttribute(kVelocityIndex, graphics::TypeId::Float, false, 3, 36, offsetof(Particle, velocity));
		vao.setAttributeDivisor(kVelocityIndex, 1);
		vao.enableAttribute(kRotationIndex);
		vao.setVertexAttribute(kRotationIndex, graphics::TypeId::Float, false, 1, 36, offsetof(Particle, rotation));
		vao.setAttributeDivisor(kRotationIndex, 1);
		vao.enableAttribute(kScaleIndex);
		vao.setVertexAttribute(kScaleIndex, graphics::TypeId::Float, false, 1, 36, offsetof(Particle, scale));
		vao.setAttributeDivisor(kScaleIndex, 1);
		vao.enableAttribute(kRemainingTimeIndex);
		vao.setVertexAttribute(kRemainingTimeIndex, graphics::TypeId::Float, false, 1, 36, offsetof(Particle, remainingTime));
		vao.setAttributeDivisor(kRemainingTimeIndex, 1);
	}


	void ParticleSystemComponent::resetAnimation()
	{
		mAccumulatedTime = 0.0f;
		mParticles = {};
	}


	void ParticleSystemComponent::setEmitter(ParticleEmitterSPtr emitter)
	{
		mEmitter = emitter;
		resetAnimation();
	}


	void ParticleSystemComponent::addRenderableShader(const RenderableShaderSPtr& shader)
	{
		mShaders.emplace_back(shader);
		mParticleSystem.addTechnique(shader->getTechnique());
		mEventManager.publish(new RenderableShaderEvent(
			RenderableShaderEvent::Operation::Add, mEntity, RenderableShaderEvent::RComponentType::ParticleSystem, shader
		));
	}


	void ParticleSystemComponent::removeRenderableShader(const RenderableShaderSPtr& shader)
	{
		mShaders.erase(std::remove(mShaders.begin(), mShaders.end(), shader), mShaders.end());
		mParticleSystem.removeTechnique(shader->getTechnique());
		mEventManager.publish(new RenderableShaderEvent(
			RenderableShaderEvent::Operation::Remove, mEntity, RenderableShaderEvent::RComponentType::ParticleSystem, shader
		));
	}


	void ParticleSystemComponent::update(float elapsedTime)
	{
		if (!mEmitter) { return; }

		// Update the particle positions and retrieve the minimum and maximum positions
		glm::vec3 minPosition(std::numeric_limits<float>::max()), maxPosition(-std::numeric_limits<float>::max());
		for (std::size_t i = 0; i < mParticles.size();) {
			if (updateParticle(mParticles[i], elapsedTime)) {
				minPosition = glm::min(minPosition, mParticles[i].position);
				maxPosition = glm::max(minPosition, mParticles[i].position);
				++i;
			}
			else {
				// Remove the dead particle
				if (mParticles.size() > 1) {
					std::swap(mParticles[i], mParticles.back());
				}
				mParticles.pop_back();
			}
		}

		// Add more particles if needed
		mAccumulatedTime += elapsedTime;
		if ((mAccumulatedTime < mEmitter->duration) || mEmitter->loop) {
			std::size_t maxSimultaneousParticles = getMaxSimultaneousParticles();

			// If the emitter was changed check if we have to resize the buffers
			if (mParticles.capacity() != maxSimultaneousParticles) {
				mParticles = std::vector<Particle>(maxSimultaneousParticles);
				mParticleSystem.getInstanceVBOs()[0].resizeAndCopy(mParticles.data(), mParticles.size());
				mParticles.clear();
			}

			std::size_t particlesLeft = (maxSimultaneousParticles >= mParticles.size())? maxSimultaneousParticles - mParticles.size() : 0;
			std::size_t particlesToAdd = std::min(static_cast<std::size_t>(RANDOM_ZERO_ONE() * particlesLeft * elapsedTime), particlesLeft);
			for (std::size_t i = 0; i < particlesToAdd; ++i) {
				Particle& particle = addParticle();
				minPosition = glm::min(minPosition, particle.position);
				maxPosition = glm::max(minPosition, particle.position);
			}
		}

		// Update the bounds
		glm::vec3 minMeshBounds(0.0f), maxMeshBounds(0.0f);
		if (mParticleSystem.getMesh()) {
			std::tie(minMeshBounds, maxMeshBounds) = mParticleSystem.getMesh()->getBounds();
			minMeshBounds *= mEmitter->scale + mEmitter->initialScaleRandomFactor;
			maxMeshBounds *= mEmitter->scale + mEmitter->initialScaleRandomFactor;
		}
		mParticleSystem.setBounds(minPosition + minMeshBounds, maxPosition + maxMeshBounds);

		// Update the particles VBO
		mParticleSystem.getInstanceVBOs()[0].copy(mParticles.data(), mParticles.size());
		mParticleSystem.setNumInstances(mParticles.size());
	}

// Private functions
	std::size_t ParticleSystemComponent::getMaxSimultaneousParticles() const
	{
		if (mEmitter && (mEmitter->duration > 0.0f)) {
			return static_cast<std::size_t>(std::ceil(mEmitter->maxParticles / mEmitter->duration));
		}
		return 0;
	}


	ParticleSystemComponent::Particle& ParticleSystemComponent::addParticle()
	{
		Particle& ret = mParticles.emplace_back();

		glm::vec3 initialDirection = mInitialOrientation * glm::vec3(0.0f, 0.0f, 1.0f);
		ret.position = mInitialPosition + glm::ballRand(1.0f) * mEmitter->initialPositionRandomFactor;
		ret.velocity = initialDirection * mEmitter->initialVelocity + glm::ballRand(1.0f) * mEmitter->initialVelocityRandomFactor;
		ret.rotation = (2.0f * RANDOM_ZERO_ONE() - 1.0f) * mEmitter->initialRotationRandomFactor;
		ret.scale = mEmitter->scale + (2.0f * RANDOM_ZERO_ONE() - 1.0f) * mEmitter->initialScaleRandomFactor;
		ret.remainingTime = mEmitter->lifeLength + (2.0f * RANDOM_ZERO_ONE() - 1.0f) * mEmitter->lifeLengthRandomFactor;

		return ret;
	}


	bool ParticleSystemComponent::updateParticle(Particle& particle, float elapsedTime)
	{
		particle.velocity += glm::vec3(0.0f, mEmitter->gravity * elapsedTime, 0.0f);
		particle.position += particle.velocity * elapsedTime;
		particle.remainingTime -= elapsedTime;
		return particle.remainingTime >= 0;
	}

}
