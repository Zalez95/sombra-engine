#ifndef CONSTRAINT_MANAGER_H
#define CONSTRAINT_MANAGER_H

#include "ConstraintIsland.h"

namespace se::physics {

	/**
	 * Class ConstraintManager, it's the class used for solving the physics
	 * constraints between the rigid bodies of the Physics System. It will
	 * split the constraints to solve in smaller sets called islands that
	 * will be responsible for solving them independently
	 */
	class ConstraintManager
	{
	private:	// Attributes
		/** A reference to the RigidBodyWorld that holds the RigidBodies */
		RigidBodyWorld* mParentWorld;

		/** The Constraint islands used for solving the Constraints */
		std::vector<ConstraintIsland> mIslands;

	public:		// Constraints
		/** Creates a new ConstraintManager
		 *
		 * @param	parentWorld the RigidBodyWorld that holds all the
		 *			RigidBodies to update */
		ConstraintManager(RigidBodyWorld& parentWorld) :
			mParentWorld(&parentWorld) {};

		/** Registers the given Constraint in the ConstraintManager, so the
		 * movement of the RigidBodies that it holds will be restricted.
		 *
		 * @param	constraint a pointer to the Constraint that we want to
		 *			register */
		void addConstraint(Constraint* constraint);

		/** @return	true if the ConstraintManager has any constraints inside,
		 *			false otherwise */
		bool hasConstraints() const { return !mIslands.empty(); };

		/** Iterates through all the ConstraintManager Constraints calling the
		 * given callback function
		 *
		 * @param	callback the function to call for each Constraint */
		template <typename F>
		void processConstraints(F&& callback) const;

		/** Removes the given Constraint from the ConstraintManager, so the
		 * movement of the RigidBodies that it holds won't longer be
		 * restricted.
		 *
		 * @param	constraint a pointer to the Constraint that we want to
		 *			remove */
		void removeConstraint(Constraint* constraint);

		/** Iterates through all the ConstraintManager RigidBodies calling the
		 * given callback function
		 *
		 * @param	callback the function to call for each RigidBody */
		template <typename F>
		void processRigidBodies(F&& callback) const;

		/** Iterates through all the Constraints of the ConstraintManager that
		 * containts the given RigidBody calling the given callback function
		 *
		 * @param	rigidBody a pointer to the RigidBody whose Constraints we
		 *			want to check
		 * @param	callback the function to call for each RigidBody */
		template <typename F>
		void processRigidBodyConstraints(
			RigidBody* rigidBody, F&& callback
		) const;

		/** Removes all the Constraints that constains the given RigidBody
		 * from the ConstraintManager.
		 *
		 * @param	rigidBody a pointer to the RigidBody whose Constraints we
		 *			want to remove */
		void removeRigidBody(RigidBody* rigidBody);

		/** Applies the constraints stored in the ConstraintManager
		 *
		 * @param	deltaTime the elapsed time since the last update in
		 *			seconds */
		void update(float deltaTime);
	};


	template <typename F>
	void ConstraintManager::processConstraints(F&& callback) const
	{
		for (const ConstraintIsland& island : mIslands) {
			island.processConstraints(callback);
		}
	}


	template <typename F>
	void ConstraintManager::processRigidBodies(F&& callback) const
	{
		std::vector<RigidBody*> rigidBodies;
		for (const ConstraintIsland& island : mIslands) {
			island.processRigidBodies([&](RigidBody* rb) {
				auto it = std::lower_bound(rigidBodies.begin(), rigidBodies.end(), rb);
				if ((it == rigidBodies.end()) || (*it != rb)) {
					rigidBodies.insert(it, rb);
				}
			});
		}

		for (RigidBody* rb : rigidBodies) {
			callback(rb);
		}
	}


	template <typename F>
	void ConstraintManager::processRigidBodyConstraints(
		RigidBody* rigidBody, F&& callback
	) const
	{
		for (const ConstraintIsland& island : mIslands) {
			island.processRigidBodyConstraints(rigidBody, callback);
		}
	}

}

#endif		// CONSTRAINT_MANAGER_H
