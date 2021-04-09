#include "se/physics/RigidBody.h"
#include "se/physics/forces/Gravity.h"

namespace se::physics {

	Gravity::Gravity(const float& gravity) : mGravity(gravity) {}


	void Gravity::apply(RigidBody& rigidBody)
	{
		float invertedMass = rigidBody.getConfig().invertedMass;

		if (invertedMass > 0.0f) {
			rigidBody.getData().forceSum += glm::vec3(0.0f, mGravity / invertedMass, 0.0f);
		}
	}

}
