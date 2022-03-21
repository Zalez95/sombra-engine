#ifndef PARTICLE_SYSTEM_SYSTEM_H
#define PARTICLE_SYSTEM_SYSTEM_H

#include "ECS.h"

namespace se::app {

	class Application;
	struct TransformsComponent;
	class ParticleSystemComponent;


	/**
	 * Class ParticleSystemSystem, it's a System used for updating the Entities'
	 * ParticleSystemComponents
	 */
	class ParticleSystemSystem : public ISystem
	{
	private:	// Attributes
		/** The Application that holds the GraphicsEngine used for rendering
		 * the ParticleSystems */
		Application& mApplication;

	public:		// Functions
		/** Creates a new ParticleSystemSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System */
		ParticleSystemSystem(Application& application);

		/** Class destructor */
		~ParticleSystemSystem();

		/** @copydoc ISystem::onNewComponent(Entity, const EntityDatabase::ComponentMask&, EntityDatabase::Query&) */
		virtual void onNewComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask,
			EntityDatabase::Query& query
		) override;

		/** @copydoc ISystem::onRemoveComponent(Entity, const EntityDatabase::ComponentMask&, EntityDatabase::Query&) */
		virtual void onRemoveComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask,
			EntityDatabase::Query& query
		) override;

		/** Updates the RenderableTerrains with the Entities */
		virtual void update() override;
	private:
		/** Function called when a ParticleSystemComponent is added to an Entity
		 *
		 * @param	entity the Entity that holds the ParticleSystemComponent
		 * @param	particleSystem a pointer to the new
		 *			ParticleSystemComponent
		 * @param	query the Query object used for interacting with the Entity
		 *			and its other Components */
		void onNewParticleSys(
			Entity entity, ParticleSystemComponent* particleSystem,
			EntityDatabase::Query& query
		);

		/** Function called when a ParticleSystemComponent is going to be
		 * removed from an Entity
		 *
		 * @param	entity the Entity that holds the ParticleSystemComponent
		 * @param	particleSystem a pointer to the ParticleSystemComponent
		 *			that is going to be removed
		 * @param	query the Query object used for interacting with the Entity
		 *			and its other Components */
		void onRemoveParticleSys(
			Entity entity, ParticleSystemComponent* particleSystem,
			EntityDatabase::Query& query
		);

		/** Function called when a TransformsComponent is added to an Entity
		 *
		 * @param	entity the Entity that holds the TransformsComponent
		 * @param	transforms a pointer to the new TransformsComponent
		 * @param	query the Query object used for interacting with the Entity
		 *			and its other Components */
		void onNewTransforms(
			Entity entity, TransformsComponent* transforms,
			EntityDatabase::Query& query
		);
	};

}

#endif		// PARTICLE_SYSTEM_SYSTEM_H
