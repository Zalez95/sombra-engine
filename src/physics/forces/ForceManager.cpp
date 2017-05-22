#include "ForceManager.h"
#include "Force.h"
#include "../PhysicsEntity.h"

namespace physics {
	
	void ForceManager::addEntity(PhysicsEntity* entity, Force* force)
	{
		mEntityForces.push_back({ entity, force });
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
			switch (it->mEntity->getType()) {
				case PhysicsEntityType::PARTICLE:
					it->mEntity->getParticle()->cleanForces();
					break;
				case PhysicsEntityType::RIGID_BODY:
					it->mEntity->getRigidBody()->cleanForces();
					break;
			}
		}

		// Apply the current forces
		for (auto it = mEntityForces.begin(); it != mEntityForces.end(); ++it) {
			switch (it->mEntity->getType()) {
				case PhysicsEntityType::PARTICLE:
					it->mForce->apply(it->mEntity->getParticle());
					break;
				case PhysicsEntityType::RIGID_BODY:
					it->mForce->apply(it->mEntity->getRigidBody());
					break;
			}
		}
	}

}
