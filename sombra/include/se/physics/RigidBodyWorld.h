#ifndef RIGID_BODY_WORLD_H
#define RIGID_BODY_WORLD_H

#include <mutex>
#include "../utils/ThreadPool.h"
#include "collision/CollisionDetector.h"
#include "constraints/ConstraintManager.h"
#include "CollisionSolver.h"
#include "RigidBody.h"

namespace se::physics {

	/**
	 * Struct CollisionProperties, holds all the collision detection parameters
	 */
	struct CollisionProperties
	{
		/** The maximum number of simultaneous intersecting colliders */
		std::size_t maxCollidersIntersecting = 128;

		/** The epsilon value used during the coarse collision detection step
		 * for testing intersections with the Colliders AABBs */
		float coarseEpsilon = 0.0001f;

		/** The maximum number of iterations of the collision/intersection
		 * algorithms */
		std::size_t maxIterations = 100;

		/** The threshold value needed for checking if the closest face in
		 * contact was found */
		float minFDifference = 0.00001f;

		/** The precision of the calculated Contact points */
		float contactPrecision = 0.0000001f;

		/** The minimum distance between the coordinates of two Contact used for
		 * used for checking if a contact is the same than another one */
		float contactSeparation = 0.00001f;

		/** The precision of the calculated ray casts */
		float raycastPrecision = 0.0000001f;
	};


	/**
	 * Struct ConstraintProperties, holds all the constraint resolution parameters
	 */
	struct ConstraintProperties
	{
		/** The velocity of the constraint resolution process of the
		 * collision NormalConstraints */
		float collisionBeta = 0.1f;

		/** The restitution factor of all the collision
		 * NormalConstraints */
		float collisionRestitutionFactor = 0.2f;

		/** The slop penetration value of all the collision
		 * NormalConstraints */
		float collisionSlopPenetration = 0.005f;

		/** The slop restitution value of all the collision
		 * NormalConstraints */
		float collisionSlopRestitution = 0.5f;

		/** The gravity acceleration value of all the collision
		 * FrictionConstraints */
		float frictionGravityAcceleration = 9.8f;

		/** The maximum number of iterations that the Gauss-Seidel algorithm
		 * should run for solving the Constraints */
		std::size_t maxIterations = 1;
	};


	/**
	 * Class LogHandler, it's the class that must be inherited from if someone
	 * wants to print the RigidBodyWorld traces. The default behavior is to
	 * not do anything with the traces.
	 */
	class LogHandler
	{
	public:		// Functions
		/** Class destructor */
		virtual ~LogHandler() = default;

		/** Traces error logs
		 *
		 * @param	str the trace string to log */
		virtual void error(const char* /* str */) {};

		/** Traces warning logs
		 *
		 * @param	str the trace string to log */
		virtual void warning(const char* /* str */) {};

		/** Traces info logs
		 *
		 * @param	str the trace string to log */
		virtual void info(const char* /* str */) {};

		/** Traces debug logs
		 *
		 * @param	str the trace string to log */
		virtual void debug(const char* /* str */) {};
	};


	/**
	 * Struct WorldProperties, holds all the properties of the RigidBodyWorld
	 */
	struct WorldProperties
	{
		/** The default LogHandler */
		static LogHandler sDefaultLogHandler;

		/** The bias value used for updating the RigidBodies' motion value */
		float motionBias = 0.1f;

		/** The bounds of the World */
		AABB worldAABB = { glm::vec3(-1000.0f), glm::vec3(1000.0f) };

		/** All the collision detection parameters */
		CollisionProperties collisionProperties;

		/** All the constraint resolution parameters */
		ConstraintProperties constraintProperties;

		/** The number of substeps executed per update */
		std::size_t numSubsteps = 4;

		/** The number of threads to use */
		std::size_t numThreads = 8;

		/** The log handler used for printing logs by the RigidBodyWorld */
		LogHandler* logHandler = &sDefaultLogHandler;
	};


	/**
	 * Class RigidBodyWorld, it holds all the properties, RigidBodies and
	 * Constraints of a simulation
	 */
	class RigidBodyWorld
	{
	private:	// Attributes
		/** All the properties of the RigidBodyWorld */
		const WorldProperties mProperties;

		/** The thread pool used by the RigidBodyWorld */
		utils::ThreadPool mThreadPool;

		/** The CollisionDetector used for detecting the collisions between the
		 * RigidBodies */
		CollisionDetector mCollisionDetector;

		/** The ConstraintManager of the RigidBodyWorld. We will delegate all
		 * the constraint resolution to it */
		ConstraintManager mConstraintManager;

		/** The CollisionSovler of the RigidBodyWorld. We will delegate the
		 * RigidBody collision resolution to it */
		CollisionSolver mCollisionSolver;

		/** All the RigidBodies that must be updated, sorted ascendently */
		std::vector<RigidBody*> mRigidBodies;

		/** The pointers to the Colliders of each RigidBody */
		std::vector<Collider*> mRigidBodiesColliders;

		/** The mutex used for protecting @see mRigidBodies and
		 * @see mRigidBodiesColliders */
		std::mutex mMutex;

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

		/** @return	the ThreadPool of the RigidBodyWorld */
		utils::ThreadPool& getThreadPool()
		{ return mThreadPool; };

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
