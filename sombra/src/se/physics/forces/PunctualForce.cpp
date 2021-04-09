#include "se/physics/RigidBody.h"
#include "se/physics/forces/PunctualForce.h"

namespace se::physics {

	PunctualForce::PunctualForce(const glm::vec3& value, const glm::vec3& point) :
		mValue(value), mPoint(point) {}


	void PunctualForce::apply(RigidBody& rigidBody)
	{
		rigidBody.getData().forceSum += mValue;
		rigidBody.getData().torqueSum += glm::cross(mPoint - rigidBody.getData().position, mValue);
	}

}
