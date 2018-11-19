#include <cassert>
#include <glm/gtc/matrix_transform.hpp>
#include "se/physics/RigidBody.h"

namespace se::physics {

	RigidBody::RigidBody() :
		invertedMass(0.0f), invertedInertiaTensor(0.0f),
		linearSlowDown(0.0f), angularSlowDown(0.0f),
		position(0.0f), orientation(1.0f, glm::vec3(0.0f)),
		linearVelocity(0.0), angularVelocity(0.0),
		linearAcceleration(0.0f), angularAcceleration(0.0f),
		forceSum(0.0f), torqueSum(0.0f)
	{
		updateRigidBodyData(*this);
	}


	RigidBody::RigidBody(
		float mass, float linearSlowDown,
		const glm::mat3& inertiaTensor, float angularSlowDown
	) : position(0.0f), orientation(1.0f, glm::vec3(0.0f)),
		linearVelocity(0.0), angularVelocity(0.0),
		linearAcceleration(0.0f), angularAcceleration(0.0f),
		forceSum(0.0f), torqueSum(0.0f)
	{
		assert(mass > 0.0f && "The mass must be bigger than zero");
		assert(linearSlowDown >= 0.0f && linearSlowDown <= 1.0f
			&& "The linearSlowDown should be in the range [0,1]");
		assert(angularSlowDown >= 0.0f && angularSlowDown <= 1.0f
			&& "The angularSlowDown should be in the range [0,1]");

		invertedMass			= 1.0f / mass;
		this->linearSlowDown	= linearSlowDown;
		invertedInertiaTensor	= glm::inverse(inertiaTensor);
		this->angularSlowDown	= angularSlowDown;

		updateRigidBodyData(*this);
	}


	void updateRigidBodyData(RigidBody& rigidBody)
	{
		// Update the transforms matrix of the RigidBody
		glm::mat4 translation	= glm::translate(glm::mat4(1.0f), rigidBody.position);
		glm::mat4 rotation		= glm::mat4_cast(rigidBody.orientation);
		rigidBody.transformsMatrix = translation * rotation;

		// Update the inertia tensor of the RigidBody
		glm::mat3 inverseTransformsMat3 = glm::inverse(rigidBody.transformsMatrix);
		rigidBody.invertedInertiaTensorWorld = glm::transpose(inverseTransformsMat3)
			* rigidBody.invertedInertiaTensor
			* inverseTransformsMat3;
	}

}
