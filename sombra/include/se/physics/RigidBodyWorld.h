#ifndef RIGID_BODY_WORLD_H
#define RIGID_BODY_WORLD_H

#include "collision/CollisionDetector.h"
#include "constraints/ConstraintManager.h"
#include "CollisionSolver.h"
#include "RigidBody.h"

namespace se::physics {

	/**
	 * Struct WorldProperties, holds all the properties of the RigidBodyWorld
	 */
	struct WorldProperties
	{
		/** The bias value used for updating the RigidBodies' motion value */
		float motionBias = 0.1f;

		/** The minimum bounds of the World */
		glm::vec3 minWorldAABB = glm::vec3(-1000.0f);

		/** The maximum bounds of the World */
		glm::vec3 maxWorldAABB = glm::vec3(1000.0f);

		/** The maximum number of simultaneous colliding RigidBodies */
		std::size_t maxCollidingRBs = 128;

		/** The maximum number of iterations of the collision detection
		 * algorithms */
		std::size_t maxCollisionIterations = 100;

		/** The threshold value needed for checking if the closest face in
		 * contact was found */
		float minFDifference = 0.00001f;

		/** The precision of the calculated Contact points */
		float contactPrecision = 0.0000001f;

		/** The minimum distance between the coordinates of two Contact used for
		 * used for checking if a contact is the same than another one */
		float contactSeparation = 0.00001f;

		/** The velocity of the constraint resolution process of the
		 * NormalConstraints */
		float collisionBeta = 0.1f;

		/** The restitution factor of all the NormalConstraints */
		float collisionRestitutionFactor = 0.2f;

		/** The slop penetration value of all the NormalConstraints */
		float collisionSlopPenetration = 0.005f;

		/** The slop restitution value of all the NormalConstraints */
		float collisionSlopRestitution = 0.5f;

		/** The gravity acceleration value of all the FrictionConstraints */
		float frictionGravityAcceleration = 9.8f;

		/** The maximum number of iterations that the Gauss-Seidel algorithm
		 * should run for solving the Constraints */
		std::size_t maxConstraintIterations = 10;
	};


	/**
	 * Class RigidBodyWorld, it holds all the properties, RigidBodies and
	 * Constraints of a simulation
	 */
	class RigidBodyWorld
	{
	private:	// Attributes
		friend class CollisionDetector;
		friend class ConstraintManager;
		friend class CollisionSolver;

		/** All the properties of the RigidBodyWorld */
		const WorldProperties mProperties;

		/** The CollisionDetector used for detecting the collisions between the
		 * RigidBodies */
		CollisionDetector mCollisionDetector;

		/** The ConstraintManager of the RigidBodyWorld. We will delegate all
		 * the constraint resolution to it */
		ConstraintManager mConstraintManager;

		/** The CollisionSovler of the RigidBodyWorld. We will delegate the
		 * RigidBody collision resolution to it */
		CollisionSolver mCollisionSolver;

		/** All the RigidBodies that must be updated */
		std::vector<RigidBody*> mRigidBodies;

	public:		// Functions
		/** Creates a new RigidBodyWorld
		 *
		 * @param	properties the WolrdProperties of the RigidBodyWorld */
		RigidBodyWorld(const WorldProperties& properties = WorldProperties());

		/** Class destructor */
		~RigidBodyWorld();

		/** @return	the WorldProperties of the RigidBodyWorld */
		const WorldProperties& getProperties() const
		{ return mProperties; };

		/** @return	the CollisionDetector of the RigidBodyWorld */
		CollisionDetector& getCollisionDetector()
		{ return mCollisionDetector; };

		/** @return	the ConstraintManager of the RigidBodyWorld */
		ConstraintManager& getConstraintManager()
		{ return mConstraintManager; };

		/** Adds the given RigidBody to the RigidBodyWorld so it will be updated
		 *
		 * @param	rigidBody a pointer to the RigidBody that we want to
		 *			update */
		void addRigidBody(RigidBody* rigidBody);

		/** Removes the given RigidBody from the RigidBodyWorld so it won't
		 * longer be updated
		 *
		 * @param	rigidBody a pointer to the RigidBody that we want to update
		 * @note	the Forces and Constraints that references the RigidBody
		 *			will also be removed from the RigidBodyWorld */
		void removeRigidBody(RigidBody* rigidBody);

		/** Updates the states of the RigidBodies added to the RigidBodyWorld
		 *
		 * @param	deltaTime the elapsed time since the last simulation of
		 *			the RigidBodies in seconds */
		void update(float deltaTime);
	};

}

#endif		// RIGID_BODY_WORLD_H
