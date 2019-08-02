#include <algorithm>
#include "se/physics/PhysicsEngine.h"
#include "se/physics/RigidBodyDynamics.h"

namespace se::physics {

	void PhysicsEngine::addRigidBody(RigidBody* rigidBody)
	{
		if (!rigidBody) { return; }

		updateRigidBody(rigidBody);
		mRigidBodies.push_back(rigidBody);
	}


	void PhysicsEngine::removeRigidBody(RigidBody* rigidBody)
	{
		if (!rigidBody) { return; }

		mRigidBodies.erase(
			std::remove(mRigidBodies.begin(), mRigidBodies.end(), rigidBody),
			mRigidBodies.end()
		);
	}


	void PhysicsEngine::updateRigidBody(RigidBody* rigidBody)
	{
		physics::updateTransforms(*rigidBody);
		setRigidBodySleepState(*rigidBody, false);
		rigidBody->state.set(RigidBody::State::Updated);
	}


	void PhysicsEngine::integrate(float delta)
	{
		float bias = std::pow(mBaseBias, delta);

		mForceManager.applyForces();
		for (RigidBody* rigidBody : mRigidBodies) {
			// Update the RigidBody data
			if (!rigidBody->state[RigidBody::State::Sleeping]) {
				se::physics::integrate(*rigidBody, delta);
				se::physics::updateMotion(*rigidBody, bias);
				se::physics::updateTransforms(*rigidBody);
				rigidBody->state.set(RigidBody::State::Updated);

				if (rigidBody->motion < mSleepEpsilon) {
					setRigidBodySleepState(*rigidBody, true);
				}
			}
		}
	}


	void PhysicsEngine::solveConstraints(float delta)
	{
		mConstraintManager.update(delta);
	}

// Private functions
	void PhysicsEngine::setRigidBodySleepState(RigidBody& rigidBody, bool value) const
	{
		if (value) {
			// Set the motion of the RigidBody to a larger value than
			// mSleepEpsilon to prevent it from falling sleep instantly
			rigidBody.motion = 2.0f * mSleepEpsilon;
			rigidBody.state.set(RigidBody::State::Sleeping);
		}
		else {
			rigidBody.motion = 0.0f;
			rigidBody.state.reset(RigidBody::State::Sleeping);
		}
	}

}
