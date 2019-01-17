#include <glm/gtc/matrix_transform.hpp>
#include "se/utils/Log.h"
#include "se/app/Entity.h"
#include "se/app/CollisionManager.h"

namespace se::app {

	constexpr float CollisionManager::kCollisionConstraintBeta;


	void CollisionManager::addEntity(Entity* entity, ColliderUPtr collider)
	{
		if (!entity || !collider) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added";
			return;
		}

		mCollisionDetector.addCollider(collider.get());
		mEntityMap.emplace(entity, std::move(collider));
		SOMBRA_INFO_LOG << "Entity " << entity << " added successfully";
	}


	void CollisionManager::addEntity(Entity* entity, ColliderUPtr collider, physics::RigidBody* rigidBody)
	{
		if (!entity || !collider || !rigidBody) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added";
			return;
		}

		mCollisionDetector.addCollider(collider.get());
		mColliderRigidBodyMap.emplace(collider.get(), rigidBody);
		mEntityMap.emplace(entity, std::move(collider));
		SOMBRA_INFO_LOG << "Entity " << entity << " added successfully with RigidBody " << rigidBody;
	}


	void CollisionManager::removeEntity(Entity* entity)
	{
		auto itEntity = mEntityMap.find(entity);
		if (itEntity != mEntityMap.end()) {
			mCollisionDetector.removeCollider(itEntity->second.get());
			mColliderRigidBodyMap.erase(itEntity->second.get());
			mEntityMap.erase(itEntity);
			SOMBRA_INFO_LOG << "Entity " << entity << " removed successfully";
		}
		else {
			SOMBRA_WARN_LOG << "Entity " << entity << " wasn't removed";
		}
	}


	void CollisionManager::update(float delta)
	{
		SOMBRA_INFO_LOG << "Updating the CollisionManager (" << delta << ")";

		// Update the Colliders with the changes made to the Entities
		for (auto& pair : mEntityMap) {
			glm::mat4 translation	= glm::translate(glm::mat4(1.0f), pair.first->position);
			glm::mat4 rotation		= glm::mat4_cast(pair.first->orientation);
			glm::mat4 scale			= glm::scale(glm::mat4(1.0f), pair.first->scale);
			pair.second->setTransforms(translation * rotation * scale);
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
						SOMBRA_INFO_LOG << "Added normal constraint " << &it->second
							<< " between " << itRB1->second << " and " << itRB2->second;
					}
				}

				if (it != mContactConstraints.end()) {
					SOMBRA_INFO_LOG << "Updating normal constraint " << &it->second;

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
				SOMBRA_INFO_LOG << "Removing normal constraint " << &it->second;
				mPhysicsEngine.getConstraintManager().removeConstraint(&it->second);
				it = mContactConstraints.erase(it);
			}
			else {
				++it;
			}
		}

		SOMBRA_INFO_LOG << "CollisionManager updated";
	}

}
