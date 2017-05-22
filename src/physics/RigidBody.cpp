#include "RigidBody.h"
#include <glm/gtc/matrix_transform.hpp>

namespace physics {


	RigidBody::RigidBody(
		const glm::vec3& position,
		const glm::quat& orientation
	) : mLinearData({}), mAngularData({})
	{
		mLinearData.mPosition		= position;
		mAngularData.mOrientation	= orientation;
	}


	RigidBody::RigidBody(
		float mass, float linearSlowDown,
		const glm::mat3& inertiaTensor, float angularSlowDown,
		const glm::vec3& position, const glm::quat& orientation
	) : mLinearData({}), mAngularData({})
	{
		assert(mass > 0);

		mLinearData.mInvertedMass			= 1.0f / mass;
		mLinearData.mSlowDown				= linearSlowDown;	// TODO: SlowDown to [0-1]?
		mLinearData.mPosition				= position;
		mAngularData.mInvertedInertiaTensor = glm::inverse(inertiaTensor);
		mAngularData.mSlowDown				= angularSlowDown;	// TODO: SlowDown to [0-1]?
		mAngularData.mOrientation			= orientation;

		updateTransformsMatrix();
		updateInertiaTensorWorld();
	}


	void RigidBody::addForce(const glm::vec3& force)
	{
		mLinearData.mForceSum += force;
	}


	void RigidBody::addForceAtPoint(const glm::vec3& force, const glm::vec3& point)
	{
		mLinearData.mForceSum += force;
		mAngularData.mTorqueSum += point * force;
	}


	void RigidBody::addForceAtLocalPoint(const glm::vec3& force, const glm::vec3& point)
	{
		glm::vec3 pointWorld = glm::mat3(mTransformsMatrix) * point + glm::vec3(mTransformsMatrix[3]);
		addForceAtPoint(force, pointWorld);
	}


	void RigidBody::cleanForces()
	{
		mLinearData.mForceSum	= glm::vec3();
		mAngularData.mTorqueSum	= glm::vec3();
	}


	void RigidBody::integrate(float delta)
	{
		// Update the Position
		glm::vec3 linearAcceleration = mLinearData.mInvertedMass * mLinearData.mForceSum;
		mLinearData.mVelocity *= glm::pow(mLinearData.mSlowDown, delta);
		mLinearData.mVelocity += linearAcceleration * delta;
		mLinearData.mPosition += mLinearData.mVelocity * delta;

		// Update the Orientation
		glm::vec3 angularAcceleration = mAngularData.mInvertedInertiaTensorWorld * mAngularData.mTorqueSum;
		mAngularData.mVelocity *= glm::pow(mAngularData.mSlowDown, delta);
		mAngularData.mVelocity += angularAcceleration * delta;
		mAngularData.mOrientation *= glm::angleAxis(delta, mAngularData.mVelocity);
		mAngularData.mOrientation = glm::normalize(mAngularData.mOrientation);

		// Update the derived data
		updateTransformsMatrix();
		updateInertiaTensorWorld();
	}

// Private functions
	void RigidBody::updateTransformsMatrix()
	{
		glm::mat4 translation	= glm::translate(glm::mat4(), mLinearData.mPosition);
		glm::mat4 rotation		= glm::mat4_cast(mAngularData.mOrientation);
		mTransformsMatrix		= translation * rotation;
	}


	void RigidBody::updateInertiaTensorWorld()
	{
		glm::mat3 transformsMat3(mTransformsMatrix);

		mAngularData.mInvertedInertiaTensorWorld = glm::transpose(inverse(transformsMat3))
			* mAngularData.mInvertedInertiaTensor
			* glm::inverse(transformsMat3);
	}

}
