#include "se/physics/RigidBody.h"
#include "se/physics/forces/Gravity.h"

namespace se::physics {

	Gravity::Gravity(const glm::vec3& gravity) : mGravity(gravity) {}


	bool Gravity::isConstant() const
	{
		return true;
	}


	void Gravity::apply(RigidBody* rigidBody)
	{
		float invertedMass = rigidBody->getConfig().invertedMass;

		if (invertedMass > 0.0f) {
			rigidBody->getData().forceSum += mGravity / invertedMass;
		}
	}

}
