#include <cmath>
#include <limits>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>
#include "se/app/ParticleSystemComponent.h"
#include "se/app/events/EventManager.h"
#include "se/app/events/RenderableShaderEvent.h"

#define RANDOM_ZERO_ONE() (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX))

namespace se::app {

	struct ParticleSystemComponent::Particle
	{
		glm::vec3 position = glm::vec3(0.0f);
		glm::vec3 velocity = glm::vec3(0.0f);
		float rotation = 0.0f;
		float scale = 0.0f;
		float remainingTime = 0.0f;
	};


	struct ParticleSystemComponent::ParticlesState
	{
		/** All the Particles that are currently alive */
		std::vector<Particle> particles;

		/** The initial location of the new Particles */
		glm::vec3 initialPosition = glm::vec3(0.0f);

		/** The initial orientation of the new Particles */
		glm::quat initialOrientation = glm::quat(1.0f, glm::vec3(0.0f));

		/** The accumulated time since the start of the particle simulation */
		float accumulatedTime = 0.0f;

		/** The bounds of ParticleSystemComponent's mMesh */
		std::pair<glm::vec3, glm::vec3> meshBounds =
			{ glm::vec3(0.0f), glm::vec3(0.0f) };

		/** The mutex used for accessing the properties of the object */
		std::mutex mutex;
	};


	ParticleSystemComponent::ParticleSystemComponent(const ParticleSystemComponent& other) :
		mParticleSystem(other.mParticleSystem),
		mMesh(other.mMesh), mEmitter(other.mEmitter), mShaders(other.mShaders)
	{
		if (other.mParticlesState) {
			mParticlesState = std::make_shared<ParticlesState>();

			std::scoped_lock lock(other.mParticlesState->mutex);
			mParticlesState->particles = other.mParticlesState->particles;
			mParticlesState->initialPosition = other.mParticlesState->initialPosition;
			mParticlesState->initialOrientation = other.mParticlesState->initialOrientation;
			mParticlesState->accumulatedTime = other.mParticlesState->accumulatedTime;
			mParticlesState->meshBounds = other.mParticlesState->meshBounds;
		}

		processRenderableShaders([&](const RenderableShaderResource& shader) {
			shader->processSteps([&](const RenderableShader::StepResource& step) {
				mParticleSystem.clearBindables(step->getPass().get());
			});
		});
	}


	ParticleSystemComponent& ParticleSystemComponent::operator=(const ParticleSystemComponent& other)
	{
		mEventManager = nullptr;
		mEntity = kNullEntity;
		mParticleSystem = other.mParticleSystem;
		mShaders = other.mShaders;
		mMesh = other.mMesh;
		mEmitter = other.mEmitter;

		if (other.mParticlesState) {
			mParticlesState = std::make_shared<ParticlesState>();

			std::scoped_lock lock(other.mParticlesState->mutex);
			mParticlesState->particles = other.mParticlesState->particles;
			mParticlesState->initialPosition = other.mParticlesState->initialPosition;
			mParticlesState->initialOrientation = other.mParticlesState->initialOrientation;
			mParticlesState->accumulatedTime = other.mParticlesState->accumulatedTime;
			mParticlesState->meshBounds = other.mParticlesState->meshBounds;
		}

		processRenderableShaders([&](const RenderableShaderResource& shader) {
			shader->processSteps([&](const RenderableShader::StepResource& step) {
				mParticleSystem.clearBindables(step->getPass().get());
			});
		});

		return *this;
	}


	void ParticleSystemComponent::setup(EventManager* eventManager, graphics::Context* context, Entity entity)
	{
		mEventManager = eventManager;
		mEntity = entity;

		if (!mParticlesState) {
			mParticlesState = std::make_shared<ParticlesState>();
		}

		if (!mGraphicsParticles) {
			mGraphicsParticles = context->create<graphics::Particles>()
				.edit([](graphics::Particles& particles) {
					auto vao = std::make_unique<graphics::VertexArray>();
					vao->enableAttribute(kPositionIndex);
					vao->setVertexAttribute(kPositionIndex, graphics::TypeId::Float, false, 3, 36, offsetof(Particle, position));
					vao->setAttributeDivisor(kPositionIndex, 1);
					vao->enableAttribute(kVelocityIndex);
					vao->setVertexAttribute(kVelocityIndex, graphics::TypeId::Float, false, 3, 36, offsetof(Particle, velocity));
					vao->setAttributeDivisor(kVelocityIndex, 1);
					vao->enableAttribute(kRotationIndex);
					vao->setVertexAttribute(kRotationIndex, graphics::TypeId::Float, false, 1, 36, offsetof(Particle, rotation));
					vao->setAttributeDivisor(kRotationIndex, 1);
					vao->enableAttribute(kScaleIndex);
					vao->setVertexAttribute(kScaleIndex, graphics::TypeId::Float, false, 1, 36, offsetof(Particle, scale));
					vao->setAttributeDivisor(kScaleIndex, 1);
					vao->enableAttribute(kRemainingTimeIndex);
					vao->setVertexAttribute(kRemainingTimeIndex, graphics::TypeId::Float, false, 1, 36, offsetof(Particle, remainingTime));
					vao->setAttributeDivisor(kRemainingTimeIndex, 1);

					std::vector<std::unique_ptr<graphics::VertexBuffer>> vbos;
					auto vbo = std::make_unique<graphics::VertexBuffer>();
					vao->bind();
					vbo->bind();
					vbos.push_back(std::move(vbo));

					particles.setBuffers(std::move(vbos), std::move(vao));
				});
			mParticleSystem.setParticles(mGraphicsParticles);
		}
	}


	glm::vec3 ParticleSystemComponent::getInitialPosition() const
	{
		if (mParticlesState) {
			std::scoped_lock lock(mParticlesState->mutex);
			return mParticlesState->initialPosition;
		}
		return glm::vec3(0.0f);
	}


	void ParticleSystemComponent::setInitialPosition(const glm::vec3& initialPosition)
	{
		if (mParticlesState) {
			std::scoped_lock lock(mParticlesState->mutex);
			mParticlesState->initialPosition = initialPosition;
		}
	}


	glm::quat ParticleSystemComponent::getInitialOrientation() const
	{
		if (mParticlesState) {
			std::scoped_lock lock(mParticlesState->mutex);
			return mParticlesState->initialOrientation;
		}
		return glm::quat(1.0f, glm::vec3(0.0f));
	}


	void ParticleSystemComponent::setInitialOrientation(const glm::quat& initialOrientation)
	{
		if (mParticlesState) {
			std::scoped_lock lock(mParticlesState->mutex);
			mParticlesState->initialOrientation = initialOrientation;
		}
	}


	void ParticleSystemComponent::setMesh(const MeshResource& mesh)
	{
		mMesh = mesh;
		mParticleSystem.setMesh(*mesh);

		if (auto pState = mParticlesState) {
			if (mMesh) {
				mMesh->edit([=](graphics::Mesh& m) {
					std::scoped_lock lock(pState->mutex);
					pState->meshBounds = m.getBounds();
				});
			}
			else {
				std::scoped_lock lock(pState->mutex);
				pState->meshBounds = { glm::vec3(0.0f), glm::vec3(0.0f) };
			}
		}
	}


	void ParticleSystemComponent::setEmitter(const ParticleEmitterResource& emitter)
	{
		mEmitter = emitter;
		resetAnimation();
	}


	void ParticleSystemComponent::addRenderableShader(const RenderableShaderResource& shader)
	{
		mShaders.emplace_back(shader);
		mParticleSystem.addTechnique(shader->getTechnique());
		if (mEventManager) {
			mEventManager->publish(std::make_unique<RenderableShaderEvent>(
				RenderableShaderEvent::Operation::Add, mEntity, RenderableShaderEvent::RComponentType::ParticleSystem, shader.get()
			));
		}
	}


	void ParticleSystemComponent::removeRenderableShader(const RenderableShaderResource& shader)
	{
		if (mEventManager) {
			mEventManager->publish(std::make_unique<RenderableShaderEvent>(
				RenderableShaderEvent::Operation::Remove, mEntity, RenderableShaderEvent::RComponentType::ParticleSystem, shader.get()
			));
		}
		mParticleSystem.removeTechnique(shader->getTechnique());
		mShaders.erase(std::remove(mShaders.begin(), mShaders.end(), shader), mShaders.end());
	}


	void ParticleSystemComponent::resetAnimation()
	{
		if (mParticlesState) {
			std::scoped_lock lock(mParticlesState->mutex);
			mParticlesState->accumulatedTime = 0.0f;
			mParticlesState->particles = {};
		}
	}


	void ParticleSystemComponent::update(float elapsedTime)
	{
		if (!mEmitter) { return; }

		std::scoped_lock lock(mParticlesState->mutex);

		// Update the particle positions and retrieve the minimum and maximum positions
		glm::vec3 minPosition(std::numeric_limits<float>::max()), maxPosition(-std::numeric_limits<float>::max());
		for (std::size_t i = 0; i < mParticlesState->particles.size();) {
			if (updateParticle(mParticlesState->particles[i], elapsedTime)) {
				minPosition = glm::min(minPosition, mParticlesState->particles[i].position);
				maxPosition = glm::max(minPosition, mParticlesState->particles[i].position);
				++i;
			}
			else {
				// Remove the dead particles
				if (mParticlesState->particles.size() > 1) {
					std::swap(mParticlesState->particles[i], mParticlesState->particles.back());
				}
				mParticlesState->particles.pop_back();
			}
		}

		// Add more particles if needed
		mParticlesState->accumulatedTime += elapsedTime;
		if ((mParticlesState->accumulatedTime < mEmitter->duration) || mEmitter->loop) {
			std::size_t maxSimultaneousParticles = getMaxSimultaneousParticles();

			// If the emitter was changed check if we have to resize the buffers
			if (mParticlesState->particles.capacity() != maxSimultaneousParticles) {
				mParticlesState->particles = {};
				mParticlesState->particles.reserve(maxSimultaneousParticles);
			}

			std::size_t particlesLeft = (maxSimultaneousParticles >= mParticlesState->particles.size())? maxSimultaneousParticles - mParticlesState->particles.size() : 0;
			std::size_t particlesToAdd = std::min(static_cast<std::size_t>(RANDOM_ZERO_ONE() * particlesLeft * elapsedTime), particlesLeft);
			for (std::size_t i = 0; i < particlesToAdd; ++i) {
				Particle& particle = addParticle();
				minPosition = glm::min(minPosition, particle.position);
				maxPosition = glm::max(minPosition, particle.position);
			}
		}

		// Update the bounds
		glm::vec3 minMeshBounds = mParticlesState->meshBounds.first, maxMeshBounds = mParticlesState->meshBounds.second;
		if (mParticleSystem.getMesh()) {
			minMeshBounds *= mEmitter->scale + mEmitter->initialScaleRandomFactor;
			maxMeshBounds *= mEmitter->scale + mEmitter->initialScaleRandomFactor;
		}
		mParticleSystem.setBounds(minPosition + minMeshBounds, maxPosition + maxMeshBounds);

		// Update the particles VBO
		mParticleSystem.getParticles().edit([pState = mParticlesState](graphics::Particles& particles) {
			std::scoped_lock lock(pState->mutex);
			particles.getVBOs()[0]->resizeAndCopy(pState->particles.data(), pState->particles.size());
		});
		mParticleSystem.setNumInstances(mParticlesState->particles.size());
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
		Particle& ret = mParticlesState->particles.emplace_back();

		glm::vec3 initialDirection = mParticlesState->initialOrientation * glm::vec3(0.0f, 0.0f, 1.0f);
		ret.position = mParticlesState->initialPosition + glm::ballRand(1.0f) * mEmitter->initialPositionRandomFactor;
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
