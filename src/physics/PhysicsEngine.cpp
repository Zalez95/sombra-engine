#include "PhysicsEngine.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
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
		if (entity) {
			mPhysicsEntities.push_back(entity);

			if (collision::Collider* collider = entity->getCollider()) {
				mCollisionDetector.addCollider(collider);
				mColliderEntityMap.emplace(collider, entity);
			}
		}
	}


	void PhysicsEngine::removePhysicsEntity(PhysicsEntity* entity)
	{
		mPhysicsEntities.erase(
			std::remove(mPhysicsEntities.begin(), mPhysicsEntities.end(), entity),
			mPhysicsEntities.end()
		);

		if (collision::Collider* collider = entity->getCollider()) {
			mCollisionDetector.removeCollider(collider);
			for (auto it = mColliderEntityMap.begin(); it != mColliderEntityMap.end(); ) {
				if (it->second == entity) {
					mColliderEntityMap.erase(it);
					break;
				}
				else {
					++it;
				}
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
			collision::Collider* collider = physicsEntity->getCollider();
			glm::mat4 colliderOffset = physicsEntity->getColliderOffset();
			collider->setTransforms(rigidBody->getTransformsMatrix() * colliderOffset);
		}
	}


	void PhysicsEngine::collide(float delta)
	{
		mCollisionDetector.update();
		for (collision::Manifold* manifold : mCollisionDetector.getCollisionManifolds()) {
			const collision::Collider* collider1 = manifold->getFirstCollider();
			RigidBody* rb1 = mColliderEntityMap[collider1]->getRigidBody();

			const collision::Collider* collider2 = manifold->getSecondCollider();
			RigidBody* rb2 = mColliderEntityMap[collider2]->getRigidBody();

			for (collision::Contact& contact : manifold->getContacts()) {
				mCollisionResolver.addContact(contact, rb1, rb2);
			}
		}
		mCollisionResolver.resolve(delta);
	}

}
