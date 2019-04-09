#ifndef PHYSICS_MANAGER_H
#define PHYSICS_MANAGER_H

#include <map>
#include <memory>
#include "../physics/RigidBody.h"
#include "../physics/PhysicsEngine.h"
#include "../physics/constraints/NormalConstraint.h"
#include "../collision/Manifold.h"
#include "../utils/FixedVector.h"
#include "EventManager.h"

namespace se::app {

	struct Entity;
	class CollisionEvent;


	/**
	 * Class PhysicsManager, it's a Manager used for storing and updating
	 * the Entities' physics data
	 */
	class PhysicsManager : public IEventListener
	{
	private:	// Nested types
		using RigidBodyUPtr = std::unique_ptr<physics::RigidBody>;
		using ContactConstraints = utils::FixedVector<
			physics::NormalConstraint, collision::Manifold::kMaxContacts
		>;

	private:	// Attributes
		/** The velocity of the constraint resolution process */
		static constexpr float kCollisionConstraintBeta = 5.0f;

		/** The Engine used for updating the data of the PhysicsEntities */
		physics::PhysicsEngine& mPhysicsEngine;

		/** The EventManager that will notify the events */
		EventManager& mEventManager;

		/** Maps the Entries added to the PhysicsManager and its physics data */
		std::map<Entity*, RigidBodyUPtr> mEntityRBMap;

		/** The NormalConstraints generated as a consecuence of the
		 * PhysicsEntities collisions */
		std::map<const collision::Manifold*, ContactConstraints>
			mManifoldConstraints;

	public:		// Functions
		/** Creates a new PhysicsManager
		 *
		 * @param	physicsEngine a reference to the PhysicsEngine used
		 *			by the PhysicsManager
		 * @param	eventManager a reference to the EventManager that the
		 *			PhysicsManager will be subscribed to */
		PhysicsManager(
			physics::PhysicsEngine& physicsEngine,
			EventManager& eventManager
		);

		/** Class destructor */
		~PhysicsManager();

		/** Notifies the PhysicsManager of the given event
		 *
		 * @param	event the IEvent to notify */
		virtual void notify(const IEvent& event) override;

		/** Adds the given Entity to the PhysicsManager and its physics data
		 * to the manager
		 *
		 * @param	entity a pointer to the Entity to add to the PhysicsManager
		 * @param	rigidBody a pointer to the RigidBody to add to the
		 *			PhysicsManager
		 * @param	rigidBody the physics data of the Entity
		 * @note	The RigidBody initial data is overrided by the Entity one */
		void addEntity(Entity* entity, RigidBodyUPtr rigidBody);

		/** Removes the given Entity from the PhysicsManager so it won't
		 * longer be updated
		 *
		 * @param	entity a pointer to the Entity to remove from the
		 *			PhysicsManager */
		void removeEntity(Entity* entity);

		/** Integrates the RigidBodies data of the entities
		 *
		 * @param	delta the elapsed time since the last update in seconds */
		void doDynamics(float delta);

		/** Solves the Constraints between the RigidBodies of the entities
		 *
		 * @param	delta the elapsed time since the last update in seconds */
		void doConstraints(float delta);
	private:
		/** Handles the given CollisionEvent by creating ContactConstraints
		 * between the colliding Entities
		 *
		 * @param	event the CollisionEvent to handle */
		void onCollisionEvent(const CollisionEvent& event);

		/** Adds contact constraints to the PhysicsManager for resolving the
		 * collision detected in the given contact Manifold
		 *
		 * @param	rb1 a pointer to the first RigidBody of the Constraints
		 * @param	rb1 a pointer to the second RigidBody of the Constraints
		 * @param	manifold a pointer to the contact Manifold */
		void handleIntersectingManifold(
			physics::RigidBody* rb1, physics::RigidBody* rb2,
			const collision::Manifold* manifold
		);

		/** Removes all the Manifold contact constraints from the PhysicsManager
		 *
		 * @param	rb1 a pointer to the first RigidBody of the Constraints
		 * @param	rb1 a pointer to the second RigidBody of the Constraints
		 * @param	manifold a pointer to the contact Manifold */
		void handleDisjointManifold(
			physics::RigidBody* rb1, physics::RigidBody* rb2,
			const collision::Manifold* manifold
		);
	};

}

#endif	// PHYSICS_MANAGER_H
