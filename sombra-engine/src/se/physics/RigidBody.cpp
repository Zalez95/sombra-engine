#include <cassert>
#include <glm/gtc/matrix_transform.hpp>
#include "se/physics/RigidBody.h"

namespace se::physics {

	RigidBody::RigidBody() :
		invertedMass(0.0f), invertedInertiaTensor(0.0f),
		linearSlowDown(0.0f), angularSlowDown(0.0f), frictionCoefficient(0.0f),
		position(0.0f), orientation(1.0f, glm::vec3(0.0f)),
		linearVelocity(0.0), angularVelocity(0.0),
		linearAcceleration(0.0f), angularAcceleration(0.0f),
		forceSum(0.0f), torqueSum(0.0f), transformsMatrix(1.0f), motion(0.0f) {}


	RigidBody::RigidBody(
		float mass, float linearSlowDown,
		const glm::mat3& inertiaTensor, float angularSlowDown,
		float frictionCoefficient
	) : position(0.0f), orientation(1.0f, glm::vec3(0.0f)),
		linearVelocity(0.0), angularVelocity(0.0),
		linearAcceleration(0.0f), angularAcceleration(0.0f),
		forceSum(0.0f), torqueSum(0.0f), transformsMatrix(1.0f), motion(0.0f)
	{
		assert(mass > 0.0f && "The mass must be larger than zero");
		assert(linearSlowDown >= 0.0f && linearSlowDown <= 1.0f
			&& "The linearSlowDown should be in the range [0,1]");
		assert(angularSlowDown >= 0.0f && angularSlowDown <= 1.0f
			&& "The angularSlowDown should be in the range [0,1]");
		assert(frictionCoefficient >= 0.0f && "The frictionCoefficient must be larger or equal than zero");

		invertedMass				= 1.0f / mass;
		this->linearSlowDown		= linearSlowDown;
		invertedInertiaTensor		= glm::inverse(inertiaTensor);
		this->angularSlowDown		= angularSlowDown;
		this->frictionCoefficient	= frictionCoefficient;
	}

}
