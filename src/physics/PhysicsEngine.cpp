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
			collision::Collider* collider = entity->getCollider();

			mPhysicsEntities.push_back(entity);
			mColliderEntityMap.emplace(collider, entity);
		}
	}


	void PhysicsEngine::removePhysicsEntity(PhysicsEntity* entity)
	{
		mPhysicsEntities.erase(
			std::remove(mPhysicsEntities.begin(), mPhysicsEntities.end(), entity),
			mPhysicsEntities.end()
		);

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
		for (PhysicsEntity* physicsEntity : mPhysicsEntities) {
			mCoarseCollisionDetector.submit( physicsEntity->getCollider() );
		}
		auto intersectingColliders = mCoarseCollisionDetector.getIntersectingColliders();

		for (std::pair<const collision::Collider*, const collision::Collider*> pair : intersectingColliders) {
			const collision::Collider* collider1 = pair.first;
			RigidBody* rb1 = mColliderEntityMap[collider1]->getRigidBody();

			const collision::Collider* collider2 = pair.second;
			RigidBody* rb2 = mColliderEntityMap[collider2]->getRigidBody();

			std::vector<collision::Contact> contacts = mFineCollisionDetector.collide(*collider1, *collider2);
			for (collision::Contact& contact : contacts) {
				mCollisionResolver.addContact(contact, rb1, rb2);
			}
		}
		mCollisionResolver.resolve(delta);
	}

}
