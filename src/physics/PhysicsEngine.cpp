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
		for (auto it = mPhysicsEntities.begin(); it != mPhysicsEntities.end(); ++it) {
			Collider* collider			= (*it)->getCollider();
			glm::mat4 colliderOffset	= (*it)->getColliderOffset();
            switch ((*it)->getType()) {
				case PhysicsEntityType::PARTICLE:
				{
					Particle* particle = (*it)->getParticle();
					particle->integrate(delta);
					collider->setTransforms(particle->getTransformsMatrix() * colliderOffset);
					break;
	            }
				case PhysicsEntityType::RIGID_BODY:
				{
					RigidBody* rigidBody = (*it)->getRigidBody();
					rigidBody->integrate(delta);
					collider->setTransforms(rigidBody->getTransformsMatrix() * colliderOffset);
					break;
				}
			}
		}
	}


	void PhysicsEngine::collide(float delta)
	{
		// TODO: coarseCollisionDetection();
		// TODO: particle collision
		for (auto it1 = mPhysicsEntities.begin(); it1 != mPhysicsEntities.end(); ++it1) {
			for (auto it2 = it1 + 1; it2 != mPhysicsEntities.end(); ++it2) {
				if (((*it1)->getType() == PhysicsEntityType::RIGID_BODY) &&
					((*it2)->getType() == PhysicsEntityType::RIGID_BODY)
				) {
					const Collider* collider1 = (*it1)->getCollider();
					const Collider* collider2 = (*it2)->getCollider();

        	        std::vector<Contact> contacts = mCollisionDetector.collide(collider1, collider2);
            	    for (Contact contact : contacts) {
                	    mCollisionResolver.addContact(&contact, (*it1)->getRigidBody(), (*it2)->getRigidBody());
					}
				}
			}
		}
		mCollisionResolver.resolve(delta);
	}

}
