#include <cassert>
#include <glm/gtc/matrix_transform.hpp>
#include "fe/physics/RigidBody.h"

namespace fe { namespace physics {

	RigidBody::RigidBody() :
		mPosition(0.0f), mLinearVelocity(0.0), mLinearAcceleration(0.0f),
		mInvertedMass(0.0f), mLinearSlowDown(0.0f),
		mOrientation(1.0f, glm::vec3(0.0f)), mAngularVelocity(0.0), mAngularAcceleration(0.0f),
		mInvertedInertiaTensor(0.0f), mAngularSlowDown(0.0f)
	{
		cleanForces();
		updateData();
	}


	RigidBody::RigidBody(
		float mass, float linearSlowDown,
		const glm::mat3& inertiaTensor, float angularSlowDown
	) : mPosition(0.0f), mLinearVelocity(0.0), mLinearAcceleration(0.0f),
		mOrientation(1.0f, glm::vec3(0.0f)), mAngularVelocity(0.0), mAngularAcceleration(0.0f)
	{
		assert(mass > 0.0f && "The mass must be bigger than zero");
		assert(linearSlowDown >= 0.0f && linearSlowDown <= 1.0f
			&& "The linearSlowDown should be in the range [0,1]");
		assert(angularSlowDown >= 0.0f && angularSlowDown <= 1.0f
			&& "The angularSlowDown should be in the range [0,1]");

		mInvertedMass			= 1.0f / mass;
		mLinearSlowDown			= linearSlowDown;
		mInvertedInertiaTensor	= glm::inverse(inertiaTensor);
		mAngularSlowDown		= angularSlowDown;

		cleanForces();
		updateData();
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
		glm::vec3 pointWorld = mTransformsMatrix * glm::vec4(point, 1.0f);
		addForceAtPoint(force, pointWorld);
	}


	void RigidBody::cleanForces()
	{
		mForceSum	= glm::vec3(0.0f);
		mTorqueSum	= glm::vec3(0.0f);
	}


	void RigidBody::integrate(float deltaTime)
	{
		// Update the linear attributes
		applyForces();
		integrateLinearAcceleration(deltaTime);
		integrateLinearVelocity(deltaTime);

		// Update the angular attributes
		applyTorque();
		integrateAngularAcceleration(deltaTime);
		integrateAngularVelocity(deltaTime);

		updateData();
	}


	void RigidBody::applyForces()
	{
		mLinearAcceleration = mInvertedMass * mForceSum;
	}


	void RigidBody::applyTorque()
	{
		mAngularAcceleration = mInvertedInertiaTensorWorld * mTorqueSum;
	}


	void RigidBody::integrateLinearAcceleration(float deltaTime)
	{
		mLinearVelocity = mLinearVelocity * glm::pow(mLinearSlowDown, deltaTime)
			+ mLinearAcceleration * deltaTime;
	}


	void RigidBody::integrateAngularAcceleration(float deltaTime)
	{
		mAngularVelocity = mAngularVelocity * glm::pow(mAngularSlowDown, deltaTime)
			+ mAngularAcceleration * deltaTime;
	}


	void RigidBody::integrateLinearVelocity(float deltaTime)
	{
		mPosition += mLinearVelocity * deltaTime;
	}


	void RigidBody::integrateAngularVelocity(float deltaTime)
	{
		const glm::quat angularVelocityQuat(0.0f, mAngularVelocity.x, mAngularVelocity.y, mAngularVelocity.z);
		mOrientation = glm::normalize(mOrientation + (0.5f * deltaTime * angularVelocityQuat) * mOrientation);
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
		glm::mat3 inverseTransformsMat3 = glm::inverse(mTransformsMatrix);

		mInvertedInertiaTensorWorld = glm::transpose(inverseTransformsMat3)
			* mInvertedInertiaTensor
			* inverseTransformsMat3;
	}

}}
