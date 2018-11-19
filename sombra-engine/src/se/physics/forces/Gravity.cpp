#include "se/physics/RigidBody.h"
#include "se/physics/forces/Gravity.h"

namespace se::physics {

	void Gravity::apply(RigidBody* rigidBody)
	{
		if (rigidBody->invertedMass > 0.0f) {
			rigidBody->forceSum += mGravity / rigidBody->invertedMass;
		}
	}

}
