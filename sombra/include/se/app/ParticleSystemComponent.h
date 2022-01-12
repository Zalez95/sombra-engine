#ifndef PARTICLE_SYSTEM_COMPONENT_H
#define PARTICLE_SYSTEM_COMPONENT_H

#include <glm/gtc/quaternion.hpp>
#include "../graphics/3D/ParticleSystem.h"
#include "graphics/RenderableShader.h"
#include "Entity.h"

namespace se::app {

	class EventManager;


	/**
	 * Struct ParticleEmitter, it holds the properties used for updating the
	 * Particles of a ParticleSystem
	 */
	struct ParticleEmitter
	{
		/** The maximum number of particles */
		std::size_t maxParticles = 0;

		/** The time since the start of the particle simulation when no more
		 * particles will be added */
		float duration = 0.0f;

		/** If the particle simulation should loop or not */
		bool loop = false;

		/** The starting velocity of the particles */
		float initialVelocity;

		/** The randomness factor of the initial position */
		float initialPositionRandomFactor = 0.0f;

		/** The randomness factor of the initial velocity */
		float initialVelocityRandomFactor = 0.0f;

		/** The randomness factor of the initial rotation */
		float initialRotationRandomFactor = 0.0f;

		/** The scale of the Particles */
		float scale = 1.0f;

		/** The randomness factor of the initial scale */
		float initialScaleRandomFactor = 0.0f;

		/** The live length in seconds of each particle */
		float lifeLength = 0.0f;

		/** The randomness factor of the live length */
		float lifeLengthRandomFactor = 0.0f;

		/** The gravity acceleration value in the Y axis to use with the
		 * particles */
		float gravity = 0.0f;
	};


	/**
	 * Class ParticleSystemComponent,
	 */
	class ParticleSystemComponent
	{
	public:		// Nested types
		using MeshResource = Repository::ResourceRef<MeshRef>;
		using ParticleEmitterResource =
			Repository::ResourceRef<ParticleEmitter>;
		using RenderableShaderResource =
			Repository::ResourceRef<RenderableShader>;
	private:
		struct Particle;
		struct ParticlesState;

	public:		// Attributes
		static constexpr int kPositionIndex			= 4;
		static constexpr int kVelocityIndex			= 5;
		static constexpr int kRotationIndex			= 6;
		static constexpr int kScaleIndex			= 7;
		static constexpr int kRemainingTimeIndex	= 8;
	private:
		/** The EventManager used for notifying the ParticleSystemComponent
		 * changes */
		EventManager* mEventManager = nullptr;

		/** The Entity that owns the ParticleSystemComponent */
		Entity mEntity = kNullEntity;

		/** The Renderable3D used for rendering all the Particles */
		graphics::ParticleSystem mParticleSystem;

		/** The Mesh used for drawing the particles */
		MeshResource mMesh = {};

		/** The emitter that holds the particles properties */
		ParticleEmitterResource mEmitter = {};

		/** The shaders added to the ParticleSystemComponent */
		std::vector<RenderableShaderResource> mShaders;

		/** The current state of the Particles */
		std::shared_ptr<ParticlesState> mParticlesState;

		/** The graphics object that holds the particles buffers */
		graphics::Context::TBindableRef<graphics::Particles> mGraphicsParticles;

	public:		// Functions
		/** Creates a new ParticleSystemComponent
		 *
		 * @param	primitiveType the type of primitive used for rendering */
		ParticleSystemComponent(
			graphics::PrimitiveType primitiveType =
				graphics::PrimitiveType::Triangle
		) : mParticleSystem({}, {}, primitiveType) {};
		ParticleSystemComponent(const ParticleSystemComponent& other);
		ParticleSystemComponent(ParticleSystemComponent&& other) = default;

		/** Class destructor */
		~ParticleSystemComponent() = default;

		/** Assignment operator */
		ParticleSystemComponent& operator=(
			const ParticleSystemComponent& other
		);
		ParticleSystemComponent& operator=(
			ParticleSystemComponent&& other
		) = default;

		/** Sets the ParticleSystemComponent attributes
		 *
		 * @param	eventManager the new EventManager of the
		 *			ParticleSystemComponent
		 * @param	context the graphics context used for initializing the
		 *			ParticleSystem buffers
		 * @param	entity the new Entity of the ParticleSystemComponent */
		void setup(
			EventManager* eventManager, graphics::Context* context,
			Entity entity
		);

		/** @return	the ParticleSystem of the ParticleSystemComponent */
		graphics::ParticleSystem& get() { return mParticleSystem; };

		/** @return	the ParticleSystem of the ParticleSystemComponent */
		const graphics::ParticleSystem& get() const { return mParticleSystem; };

		/** @return	the initial Position of the Particles */
		glm::vec3 getInitialPosition() const;

		/** @return	the initial Position of the Particles */
		void setInitialPosition(const glm::vec3& initialPosition);

		/** @return	the initial Rotation of the Particles */
		glm::quat getInitialOrientation() const;

		/** @return	the initial Rotation of the Particles */
		void setInitialOrientation(const glm::quat& initialOrientation);

		/** @return	the Mesh of the ParticleSystem */
		const MeshResource& getMesh() const { return mMesh; };

		/** Sets the Mesh of the ParticleSystem
		 *
		 * @param	mesh the new Mesh of the ParticleSystem */
		void setMesh(const MeshResource& mesh);

		/** @return	a pointer to the ParticleEmitter of the
		 *			ParticleSystemComponent */
		const ParticleEmitterResource& getEmitter() const { return mEmitter; };

		/** Sets the ParticleEmitter of the ParticleSystemComponent
		 *
		 * @param	emitter a pointer to the new ParticleEmitter */
		void setEmitter(const ParticleEmitterResource& emitter);

		/** Adds the given RenderableShader to the ParticleSystemComponent
		 *
		 * @param	shader a pointer to the shader to add */
		void addRenderableShader(const RenderableShaderResource& shader);

		/** Iterates through all the RenderableShaders of the ParticleSystemComponents
		 * calling the given callback function
		 *
		 * @param	callback the function to call for each RenderableShader */
		template <typename F>
		void processRenderableShaders(F&& callback) const
		{
			for (auto& shader : mShaders) {
				callback(shader);
			}
		}

		/** Removes the given RenderableShader from the ParticleSystemComponent
		 *
		 * @param	shader a pointer to the shader to remove */
		void removeRenderableShader(const RenderableShaderResource& shader);

		/** Resets the particle simulation */
		void resetAnimation();

		/** Updates the properties of the Particles based on the given time,
		 * removing those that aren't alive anymore
		 *
		 * @param	elapsedTime the elapsed time in seconds since the last
		 *			update */
		void update(float elapsedTime);
	private:
		/** @return	the maximum number of particles that can exist at a single
		 *			time */
		std::size_t getMaxSimultaneousParticles() const;

		/** Adds a new Particle to the ParticleSystemComponent
		 * @note	the mParticlesState mutex should be already locked */
		Particle& addParticle();

		/** Updates a single Particle with the given time
		 *
		 * @param	particle the Particle to update
		 * @param	elapsedTime the elapsed time in seconds since the last
		 *			update
		 * @return	true if the Particle is still alive, false otherwise */
		bool updateParticle(Particle& particle, float elapsedTime);
	};

}

#endif		// PARTICLE_SYSTEM_COMPONENT_H
