#include "ForceManager.h"
#include "Force.h"
#include "../PhysicsEntity.h"

namespace physics {
	
	void ForceManager::addEntity(PhysicsEntity* entity, Force* force)
	{
		mEntityForces.emplace_back(entity, force);
	}

	
	void ForceManager::removeEntity(PhysicsEntity* entity, Force* force)
	{
		for (auto it = mEntityForces.begin(); it != mEntityForces.end(); ++it) {
			if ((it->mEntity == entity) && (it->mForce == force)) {
				mEntityForces.erase(it);
				break;
			}
		}
	}

	
	void ForceManager::applyForces()
	{
		// Clean the older forces
		for (auto it = mEntityForces.begin(); it != mEntityForces.end(); ++it) {
			it->mEntity->getRigidBody()->cleanForces();
		}

		// Apply the current forces
		for (auto it = mEntityForces.begin(); it != mEntityForces.end(); ++it) {
			it->mForce->apply(it->mEntity->getRigidBody());
		}
	}

}
