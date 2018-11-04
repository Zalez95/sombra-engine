#include "fe/physics/PhysicsEngine.h"
#include "fe/physics/RigidBody.h"
#include "fe/physics/RigidBodyDynamics.h"

namespace fe { namespace physics {

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
			fe::physics::integrate(*rigidBody, delta);
		}
	}


	void PhysicsEngine::solveConstraints(float delta)
	{
		mConstraintManager.update(delta);
	}

}}
