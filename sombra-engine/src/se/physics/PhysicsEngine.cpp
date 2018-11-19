#include "se/physics/PhysicsEngine.h"
#include "se/physics/RigidBody.h"
#include "se/physics/RigidBodyDynamics.h"

namespace se::physics {

	void PhysicsEngine::addRigidBody(RigidBody* rigidBody)
	{
		if (!rigidBody) { return; }

		mRigidBodies.emplace(rigidBody);
	}


	void PhysicsEngine::removeRigidBody(RigidBody* rigidBody)
	{
		if (!rigidBody) { return; }

		mRigidBodies.erase(rigidBody);
	}

// Private functions
	void PhysicsEngine::integrate(float delta)
	{
		mForceManager.applyForces();
		for (RigidBody* rigidBody : mRigidBodies) {
			// TODO: Non movable rigid bodies (like buildings)

			// Update the RigidBody data
			se::physics::integrate(*rigidBody, delta);
		}
	}


	void PhysicsEngine::solveConstraints(float delta)
	{
		mConstraintManager.update(delta);
	}

}
