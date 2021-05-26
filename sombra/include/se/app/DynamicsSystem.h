#ifndef DYNAMICS_SYSTEM_H
#define DYNAMICS_SYSTEM_H

#include "ECS.h"

namespace se::physics { class RigidBody; }

namespace se::app {

	class Application;


	/**
	 * Class DynamicsSystem, it's a System for updating the Entities' physics
	 * data
	 */
	class DynamicsSystem : public ISystem
	{
	private:	// Attributes
		/** The Application that holds the PhysicsEngine used for updating
		 * Entities */
		Application& mApplication;

	public:		// Functions
		/** Creates a new DynamicsSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System */
		DynamicsSystem(Application& application);

		/** Class destructor */
		~DynamicsSystem();

		/** @copydoc ISystem::onNewComponent(Entity, const EntityDatabase::ComponentMask&) */
		virtual void onNewComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask
		) override
		{ tryCallC(&DynamicsSystem::onNewRigidBody, entity, mask); };

		/** @copydoc ISystem::onRemoveComponent(Entity, const EntityDatabase::ComponentMask&) */
		virtual void onRemoveComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask
		) override
		{ tryCallC(&DynamicsSystem::onRemoveRigidBody, entity, mask); };

		/** Integrates the RigidBodies data of the entities */
		virtual void update() override;
	private:
		/** Function called when a RigidBody is added to an Entity
		 *
		 * @param	entity the Entity that holds the RigidBody
		 * @param	rigidBody a pointer to the new RigidBody */
		void onNewRigidBody(Entity entity, physics::RigidBody* rigidBody);

		/** Function called when a RigidBody is going to be removed from an
		 * Entity
		 *
		 * @param	entity the Entity that holds the RigidBody
		 * @param	rigidBody a pointer to the RigidBody that is going to be
		 *			removed */
		void onRemoveRigidBody(Entity entity, physics::RigidBody* rigidBody);
	};

}

#endif	// DYNAMICS_SYSTEM_H
