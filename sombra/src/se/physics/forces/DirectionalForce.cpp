#include "se/physics/RigidBody.h"
#include "se/physics/forces/DirectionalForce.h"

namespace se::physics {

	DirectionalForce::DirectionalForce(const glm::vec3& value) :
		mValue(value) {}


	Force::ForceTorquePair DirectionalForce::calculate(const RigidBody&) const
	{
		return { mValue, glm::vec3(0.0f) };
	}

}
