#include "fe/physics/RigidBody.h"
#include <cassert>
#include <glm/gtc/matrix_transform.hpp>

namespace fe { namespace physics {

	RigidBody::RigidBody() :
		mPosition(0.0f), mLinearVelocity(0.0), mLinearAcceleration(0.0f),
		mInvertedMass(0.0f), mLinearSlowDown(0.0f),
		mOrientation(1.0f, glm::vec3(0.0f)), mAngularVelocity(0.0), mAngularAcceleration(0.0f),
		mInvertedInertiaTensor(0.0f), mAngularSlowDown(0.0f)
	{
		cleanForces();
		updateTransformsMatrix();
		updateInertiaTensorWorld();
	}


	RigidBody::RigidBody(
		float mass, float linearSlowDown,
		const glm::mat3& inertiaTensor, float angularSlowDown
	) : mPosition(0.0f), mLinearVelocity(0.0), mLinearAcceleration(0.0f),
		mOrientation(1.0f, glm::vec3(0.0f)), mAngularVelocity(0.0), mAngularAcceleration(0.0f)
	{
		assert(mass > 0);

		mInvertedMass			= 1.0f / mass;
		mLinearSlowDown			= linearSlowDown;	// TODO: SlowDown to [0-1]?
		mInvertedInertiaTensor	= glm::inverse(inertiaTensor);
		mAngularSlowDown		= angularSlowDown;	// TODO: SlowDown to [0-1]?

		cleanForces();
		updateTransformsMatrix();
		updateInertiaTensorWorld();
	}


	void RigidBody::addForce(const glm::vec3& force)
	{
		mForceSum	+= force;
	}


	void RigidBody::addForceAtPoint(const glm::vec3& force, const glm::vec3& point)
	{
		mForceSum	+= force;
		mTorqueSum	+= glm::cross(point, force);
	}


	void RigidBody::addForceAtLocalPoint(const glm::vec3& force, const glm::vec3& point)
	{
		glm::vec3 pointWorld = glm::vec3(mTransformsMatrix[3]) + glm::mat3(mTransformsMatrix) * point;
		addForceAtPoint(force, pointWorld);
	}


	void RigidBody::cleanForces()
	{
		mForceSum	= glm::vec3(0.0f);
		mTorqueSum	= glm::vec3(0.0f);
	}


	void RigidBody::integrate(float delta)
	{
		// Update the Position
		mLinearAcceleration = mInvertedMass * mForceSum;
		mLinearVelocity = mLinearVelocity * glm::pow(mLinearSlowDown, delta) + mLinearAcceleration * delta;
		mPosition += mLinearVelocity * delta;

		// Update the Orientation
		mAngularAcceleration = mInvertedInertiaTensorWorld * mTorqueSum;
		mAngularVelocity = mAngularVelocity * glm::pow(mAngularSlowDown, delta) + mAngularAcceleration * delta;
		mOrientation = glm::normalize(0.5f * mOrientation * (mAngularVelocity * delta));

		// Update the derived data
		updateData();
	}


	void RigidBody::updateData()
	{
		updateTransformsMatrix();
		updateInertiaTensorWorld();
	}

// Private functions
	void RigidBody::updateTransformsMatrix()
	{
		glm::mat4 translation	= glm::translate(glm::mat4(1.0f), mPosition);
		glm::mat4 rotation		= glm::mat4_cast(mOrientation);
		mTransformsMatrix		= translation * rotation;
	}


	void RigidBody::updateInertiaTensorWorld()
	{
		glm::mat3 transformsMat3(mTransformsMatrix);

		mInvertedInertiaTensorWorld = glm::transpose(inverse(transformsMat3))
			* mInvertedInertiaTensor
			* glm::inverse(transformsMat3);
	}

}}
