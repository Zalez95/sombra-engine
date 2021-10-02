#ifndef COLLISION_SOLVER_H
#define COLLISION_SOLVER_H

#include <unordered_map>
#include "constraints/NormalConstraint.h"
#include "constraints/FrictionConstraint.h"
#include "collision/CollisionDetector.h"

namespace se::physics {

	class RigidBodyWorld;


	/**
	 * Class CollisionSolver, it's ICollisionListener used for creating and
	 * removing Constraints used for solving the Collisions between the
	 * RigidBodies
	 */
	class CollisionSolver : public ICollisionListener
	{
	private:	// Nested types
		/** Holds the indices of all the Constraints per Contact */
		struct ContactConstraintIndices
		{
			std::size_t iNormalConstraint;
			std::size_t iFrictionConstraints[2];
		};

		using ManifoldConstraintIndices = utils::FixedVector<
			ContactConstraintIndices, Manifold::kMaxContacts
		>;

	private:	// Attributes
		/** A reference to the RigidBodyWorld that holds the RigidBodies */
		RigidBodyWorld& mParentWorld;

		/** Maps each Manifold with the indices of the Constraints generated
		 * as a consecuence of the collisions */
		std::unordered_map<
			const Manifold*, ManifoldConstraintIndices
		> mManifoldConstraintIndicesMap;

		/** The NormalConstraints of all the Contacts */
		utils::PackedVector<NormalConstraint> mContactNormalConstraints;

		/** The FrictionConstraints of all the Contacts */
		utils::PackedVector<FrictionConstraint> mContactFrictionConstraints;

	public:		// Functions
		/** Creates a new CollisionSolver
		 *
		 * @param	parentWorld the RigidBodyWorld that holds all the
		 *			RigidBodies to update */
		CollisionSolver(RigidBodyWorld& parentWorld);

		/** Class destructor */
		~CollisionSolver();

		/** @copydoc ICollisionListener::onCollision(const Manifold&) */
		virtual void onCollision(const Manifold& manidold) override;

		/** Removes all the Manifolds and Collision Constraints that references
		 * the given RigidBody
		 *
		 * @param	rigidBody a pointer to the RigidBody to remove */
		void removeRigidBody(const RigidBody* rigidBody);

		/** Solves the Constraints between the RigidBodies of the entities
		 *
		 * @param	deltaTime the elapsed time since the last update in
		 *			seconds */
		void update(float deltaTime);
	private:
		/** Adds contact constraints to the CollisionSolver for resolving the
		 * collision detected in the given contact Manifold
		 *
		 * @param	manifold the contact Manifold */
		void handleIntersectingManifold(const Manifold& manifold);

		/** Removes all the Manifold contact constraints from the CollisionSolver
		 *
		 * @param	manifold the contact Manifold */
		void handleDisjointManifold(const Manifold& manifold);
	};

}

#endif	// COLLISION_SOLVER_H
