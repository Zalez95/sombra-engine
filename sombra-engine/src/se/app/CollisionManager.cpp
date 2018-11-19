#include <glm/gtc/matrix_transform.hpp>
#include "se/app/Entity.h"
#include "se/app/CollisionManager.h"

namespace se::app {

	constexpr float CollisionManager::kCollisionConstraintBeta;


	void CollisionManager::addEntity(Entity* entity, ColliderUPtr collider)
	{
		if (!entity || !collider) return;

		mCollisionDetector.addCollider(collider.get());
		mEntityMap[entity] = std::move(collider);
	}


	void CollisionManager::addEntity(Entity* entity, ColliderUPtr collider, physics::RigidBody* rigidBody)
	{
		if (!entity || !collider) return;

		mCollisionDetector.addCollider(collider.get());
		mColliderRigidBodyMap[collider.get()] = rigidBody;
		mEntityMap[entity] = std::move(collider);
	}


	void CollisionManager::removeEntity(Entity* entity)
	{
		auto itEntity = mEntityMap.find(entity);
		if (itEntity != mEntityMap.end()) {
			mCollisionDetector.removeCollider(itEntity->second.get());
			mColliderRigidBodyMap.erase(itEntity->second.get());
			mEntityMap.erase(itEntity);
		}
	}


	void CollisionManager::update(float delta)
	{
		// Update the Colliders with the changes made to the Entities
		for (auto& pair : mEntityMap) {
			glm::mat4 translation	= glm::translate(glm::mat4(1.0f), pair.first->position);
			glm::mat4 rotation		= glm::mat4_cast(pair.first->orientation);
			pair.second->setTransforms(translation * rotation);
		}

		mCollisionDetector.update();

		std::set<collision::Contact*> active;
		for (collision::Manifold* manifold : mCollisionDetector.getCollisionManifolds()) {
			for (collision::Contact& contact : manifold->getContacts()) {
				// Find the Constraint of the Contact or create a new one if
				// it doesn't exist yet
				auto it = mContactConstraints.find(&contact);
				if (it == mContactConstraints.end()) {
					auto itRB1 = mColliderRigidBodyMap.find(manifold->getCollider(0));
					auto itRB2 = mColliderRigidBodyMap.find(manifold->getCollider(1));

					if ((itRB1 != mColliderRigidBodyMap.end()) && (itRB2 != mColliderRigidBodyMap.end())) {
						it = mContactConstraints.emplace(
							std::piecewise_construct,
							std::forward_as_tuple(&contact),
							std::forward_as_tuple(
								std::array<physics::RigidBody*, 2>{ itRB1->second, itRB2->second },
								kCollisionConstraintBeta
							)
						).first;
						mPhysicsEngine.getConstraintManager().addConstraint(&it->second);
					}
				}

				if (it != mContactConstraints.end()) {
					// Update the constraint data
					it->second.setConstraintPoints({ contact.getLocalPosition(0), contact.getLocalPosition(1) });
					it->second.setConstraintNormal(contact.getNormal());
					it->second.setDeltaTime(delta);
					it->second.increaseK();

					// Mark the constraint as active
					active.insert(&contact);
				}
			}
		}

		// Remove the inactive contact constraints
		for (auto it = mContactConstraints.begin(); it != mContactConstraints.end();) {
			if (active.find(it->first) == active.end()) {
				mPhysicsEngine.getConstraintManager().removeConstraint(&it->second);
				it = mContactConstraints.erase(it);
			}
			else {
				++it;
			}
		}
	}

}
