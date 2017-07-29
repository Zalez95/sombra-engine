#include "PhysicsEngine.h"
#include <glm/gtc/matrix_transform.hpp>
#include "../utils/Logger.h"
#include "PhysicsEntity.h"

namespace physics {

	void PhysicsEngine::update(float delta)
	{
		mForceManager.applyForces();
		integrate(delta);
		collide(delta);
	}


	void PhysicsEngine::addPhysicsEntity(PhysicsEntity* entity)
	{
		if (entity != nullptr) {
			mPhysicsEntities.push_back(entity);
		}
	}


	void PhysicsEngine::removePhysicsEntity(PhysicsEntity* entity)
	{
		for (auto it = mPhysicsEntities.begin(); it != mPhysicsEntities.end(); ++it) {
			if ((*it) == entity) {
				mPhysicsEntities.erase(it);
				break;
			}
		}
	}

// Private functions
	void PhysicsEngine::integrate(float delta)
	{
		for (PhysicsEntity* physicsEntity : mPhysicsEntities) {
			// Update the RigidBody data
			RigidBody* rigidBody = physicsEntity->getRigidBody();
			rigidBody->integrate(delta);

			// Update the Collider data
			Collider* collider = physicsEntity->getCollider();
			glm::mat4 colliderOffset = physicsEntity->getColliderOffset();
			collider->setTransforms(rigidBody->getTransformsMatrix() * colliderOffset);
		}
	}


	void PhysicsEngine::collide(float delta)
	{
		for (auto it1 = mPhysicsEntities.begin(); it1 != mPhysicsEntities.end(); ++it1) {
			for (auto it2 = it1 + 1; it2 != mPhysicsEntities.end(); ++it2) {
				const Collider* collider1 = (*it1)->getCollider();
				const Collider* collider2 = (*it2)->getCollider();
				if (collider1 && collider2) {
        	        std::vector<Contact> contacts = mCollisionDetector.collide(*collider1, *collider2);
            	    for (Contact& contact : contacts) {
                	    mCollisionResolver.addContact(contact, (*it1)->getRigidBody(), (*it2)->getRigidBody());
					}
				}
			}
		}
		mCollisionResolver.resolve(delta);
	}

}
