#include "se/physics/RigidBody.h"
#include "se/physics/forces/DirectionalForce.h"

namespace se::physics {

	DirectionalForce::DirectionalForce(const glm::vec3& value) :
		mValue(value) {}


	void DirectionalForce::apply(RigidBody& rigidBody)
	{
		rigidBody.getData().forceSum += mValue;
	}

}
