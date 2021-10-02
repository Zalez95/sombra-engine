#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include "ECS.h"

namespace se::app {

	class Application;
	class RigidBodyComponent;


	/**
	 * Class PhysicsSystem, it's a System for updating the Entities' physics
	 * data
	 */
	class PhysicsSystem : public ISystem
	{
	private:	// Attributes
		/** The Application that holds the PhysicsEngine used for updating
		 * Entities */
		Application& mApplication;

	public:		// Functions
		/** Creates a new PhysicsSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System */
		PhysicsSystem(Application& application);

		/** Class destructor */
		~PhysicsSystem();

		/** @copydoc ISystem::onNewComponent(Entity, const EntityDatabase::ComponentMask&) */
		virtual void onNewComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask
		) override
		{ tryCallC(&PhysicsSystem::onNewRigidBody, entity, mask); };

		/** @copydoc ISystem::onRemoveComponent(Entity, const EntityDatabase::ComponentMask&) */
		virtual void onRemoveComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask
		) override
		{ tryCallC(&PhysicsSystem::onRemoveRigidBody, entity, mask); };

		/** Updates the RigidBodies of the entities */
		virtual void update() override;
	private:
		/** Function called when a RigidBodyComponent is added to an Entity
		 *
		 * @param	entity the Entity that holds the RigidBodyComponent
		 * @param	rigidBody a pointer to the new RigidBodyComponent */
		void onNewRigidBody(Entity entity, RigidBodyComponent* rigidBody);

		/** Function called when a RigidBodyComponent is going to be removed
		 * from an Entity
		 *
		 * @param	entity the Entity that holds the RigidBodyComponent
		 * @param	rigidBody a pointer to the RigidBodyComponent that is going
		 *			to be removed */
		void onRemoveRigidBody(Entity entity, RigidBodyComponent* rigidBody);
	};

}

#endif	// PHYSICS_SYSTEM_H
