#ifndef CONSTRAINTS_SYSTEM_H
#define CONSTRAINTS_SYSTEM_H

#include <unordered_map>
#include "../physics/constraints/NormalConstraint.h"
#include "../physics/constraints/FrictionConstraint.h"
#include "../collision/Manifold.h"
#include "../utils/PackedVector.h"
#include "ECS.h"

namespace se::app {

	class Application;
	class CollisionEvent;


	/**
	 * Class ConstraintsSystem, it's a System used for updating the Entities'
	 * physics data with the constraints changes
	 */
	class ConstraintsSystem : public ISystem
	{
	private:	// Nested types
		/** Holds the indices of all the Constraints per Contact */
		struct ContactConstraintIndices
		{
			std::size_t iNormalConstraint;
			std::size_t iFrictionConstraints[2];
		};

		using ManifoldConstraintIndices = utils::FixedVector<
			ContactConstraintIndices, collision::Manifold::kMaxContacts
		>;

	private:	// Attributes
		/** The velocity of the constraint resolution process of the
		 * NormalConstraints */
		static constexpr float kCollisionBeta = 0.1f;

		/** The restitution factor of all the NormalConstraints */
		static constexpr float kCollisionRestitutionFactor = 0.2f;

		/** The slop penetration value of all the NormalConstraints */
		static constexpr float kCollisionSlopPenetration = 0.005f;

		/** The slop restitution value of all the NormalConstraints */
		static constexpr float kCollisionSlopRestitution = 0.5f;

		/** The gravity acceleration value of all the FrictionConstraints */
		static constexpr float kFrictionGravityAcceleration = 9.8f;

		/** The maximum number of Contacts that can be solved at a single
		 * time */
		static constexpr std::size_t kMaxContacts = 64000;

		/** The Application that holds the PhysicsEngine and the EventManager
		 * used for updating Entities' RigidBodies and being notified of the
		 * Collisions */
		Application& mApplication;

		/** Maps each Manifold with the indices of the Constraints generated
		 * as a consecuence of the collisions */
		std::unordered_map<
			const collision::Manifold*, ManifoldConstraintIndices
		> mManifoldConstraintIndicesMap;

		/** The NormalConstraints of all the Contacts */
		utils::PackedVector<
			physics::NormalConstraint
		> mContactNormalConstraints;

		/** The FrictionConstraints of all the Contacts */
		utils::PackedVector<
			physics::FrictionConstraint
		> mContactFrictionConstraints;

	public:		// Functions
		/** Creates a new ConstraintsSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System */
		ConstraintsSystem(Application& application);

		/** Class destructor */
		~ConstraintsSystem();

		/** @copydoc ISystem::notify(const IEvent&) */
		virtual void notify(const IEvent& event) override;

		/** @copydoc ISystem::onNewComponent(Entity, const EntityDatabase::ComponentMask&) */
		virtual void onNewComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask
		) override
		{ tryCallC(&ConstraintsSystem::onNewRigidBody, entity, mask); };

		/** @copydoc ISystem::onRemoveComponent(Entity, const EntityDatabase::ComponentMask&) */
		virtual void onRemoveComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask
		) override
		{ tryCallC(&ConstraintsSystem::onRemoveRigidBody, entity, mask); };

		/** Solves the Constraints between the RigidBodies of the entities
		 *
		 * @param	delta the elapsed time since the last update in seconds */
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

		/** Handles the given CollisionEvent by creating ContactConstraints
		 * between the colliding Entities
		 *
		 * @param	event the CollisionEvent to handle */
		void onCollisionEvent(const CollisionEvent& event);

		/** Adds contact constraints to the ConstraintsSystem for resolving the
		 * collision detected in the given contact Manifold
		 *
		 * @param	rb1 a pointer to the first RigidBody of the Constraints
		 * @param	rb1 a pointer to the second RigidBody of the Constraints
		 * @param	manifold a pointer to the contact Manifold */
		void handleIntersectingManifold(
			physics::RigidBody* rb1, physics::RigidBody* rb2,
			const collision::Manifold* manifold
		);

		/** Removes all the Manifold contact constraints from the ConstraintsSystem
		 *
		 * @param	manifold a pointer to the contact Manifold */
		void handleDisjointManifold(const collision::Manifold* manifold);
	};

}

#endif	// CONSTRAINTS_SYSTEM_H
