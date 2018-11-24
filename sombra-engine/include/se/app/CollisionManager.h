#ifndef COLLISION_MANAGER_H
#define COLLISION_MANAGER_H

#include <map>
#include <memory>
#include "../collision/Collider.h"
#include "../collision/CollisionDetector.h"
#include "../physics/PhysicsEngine.h"
#include "../physics/constraints/NormalConstraint.h"

namespace se::app {

	struct Entity;


	/**
	 * Class CollisionManager, it's an Manager used for storing and updating
	 * the Entities' collision data
	 */
	class CollisionManager
	{
	private:	// Nested types
		using ColliderUPtr = std::unique_ptr<collision::Collider>;

	private:	// Attributes
		/** The velocity of the constraint resolution process */
		static constexpr float kCollisionConstraintBeta = 5.0f;

		/** Maps the Entries added to the CollisionManager and its Collider
		 * data */
		std::map<Entity*, ColliderUPtr> mEntityMap;

		/** The CollisionDetector used for detecting the collisions the data of
		 * the Colliders */
		collision::CollisionDetector& mCollisionDetector;

		/** The PhysicsEngine used for solving the collisions between the
		 * entities */
		physics::PhysicsEngine& mPhysicsEngine;

		/** Maps each Collider with its RigidBody */
		std::map<const collision::Collider*, physics::RigidBody*>
			mColliderRigidBodyMap;

		/** The NormalConstraints generated as a consecuence of the
		 * PhysicsEntities collisions */
		std::map<collision::Contact*, physics::NormalConstraint>
			mContactConstraints;

	public:		// Functions
		/** Creates a new CollisionManager */
		CollisionManager(
			collision::CollisionDetector& collisionDetector,
			physics::PhysicsEngine& physicsEngine
		) : mCollisionDetector(collisionDetector),
			mPhysicsEngine(physicsEngine) {};

		/** Adds the given Entity to the CollisionManager and its collider data
		 * to the manager
		 *
		 * @param	entity a pointer to the Entity to add to the
		 *			CollisionManager
		 * @param	collider the collider data of the Entity */
		void addEntity(Entity* entity, ColliderUPtr collider);

		/** Adds the given Entity to the CollisionManager and its collider data
		 * to the manager
		 *
		 * @param	entity a pointer to the Entity to add to the
		 *			CollisionManager
		 * @param	collider the collider data of the Entity
		 * @param	rigidBody the physics data of the Entity */
		void addEntity(
			Entity* entity,
			ColliderUPtr collider, physics::RigidBody* rigidBody
		);

		/** Removes the given Entity from the CollisionManager so it won't
		 * longer be updated
		 *
		 * @param	entity a pointer to the Entity to remove from the
		 *			CollisionManager */
		void removeEntity(Entity* entity);

		/** Detects the collisions between the entities' colliders
		 *
		 * @param	delta the elapsed time since the last update in seconds */
		void update(float delta);
	};

}

#endif	// COLLISION_MANAGER_H
