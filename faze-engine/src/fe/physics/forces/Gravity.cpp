#include "fe/physics/RigidBody.h"
#include "fe/physics/forces/Gravity.h"

namespace fe { namespace physics {

	void Gravity::apply(RigidBody* rigidBody)
	{
		if (rigidBody->invertedMass > 0) {
			rigidBody->forceSum += mGravity / rigidBody->invertedMass;
		}
	}

}}
