#include "fe/physics/forces/ForceManager.h"
#include "fe/physics/forces/Force.h"
#include "fe/physics/PhysicsEntity.h"

namespace fe { namespace physics {

	void ForceManager::addEntity(PhysicsEntity* entity, Force* force)
	{
		mEntityForces.emplace_back(entity, force);
	}


	void ForceManager::removeEntity(PhysicsEntity* entity, Force* force)
	{
		for (auto it = mEntityForces.begin(); it != mEntityForces.end(); ++it) {
			if ((it->entity == entity) && (it->force == force)) {
				mEntityForces.erase(it);
				break;
			}
		}
	}


	void ForceManager::applyForces()
	{
		// Clean the older forces
		for (auto it = mEntityForces.begin(); it != mEntityForces.end(); ++it) {
			it->entity->getRigidBody()->cleanForces();
		}

		// Apply the current forces
		for (auto it = mEntityForces.begin(); it != mEntityForces.end(); ++it) {
			it->force->apply(it->entity->getRigidBody());
		}
	}

}}
