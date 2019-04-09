#include <glm/gtc/matrix_transform.hpp>
#include "se/utils/Log.h"
#include "se/app/Entity.h"
#include "se/app/CollisionManager.h"
#include "se/app/events/CollisionEvent.h"

namespace se::app {

	CollisionManager::CollisionManager(collision::CollisionWorld& collisionWorld, EventManager& eventManager) :
		mCollisionWorld(collisionWorld), mEventManager(eventManager) {}


	void CollisionManager::addEntity(Entity* entity, ColliderUPtr collider)
	{
		if (!entity || !collider) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added";
			return;
		}

		collision::Collider* cPtr = collider.get();
		mCollisionWorld.addCollider(cPtr);
		mColliderEntityMap.emplace(cPtr, entity);
		mEntityColliderMap.emplace(entity, std::move(collider));
		SOMBRA_INFO_LOG << "Entity " << entity << " with Collider " << cPtr << " added successfully";
	}


	void CollisionManager::removeEntity(Entity* entity)
	{
		auto itPair = mEntityColliderMap.find(entity);
		if (itPair != mEntityColliderMap.end()) {
			mCollisionWorld.removeCollider( itPair->second.get() );
			mColliderEntityMap.erase( mColliderEntityMap.find(itPair->second.get()) );
			mEntityColliderMap.erase(itPair);
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
		for (auto& pair : mEntityColliderMap) {
			glm::mat4 translation	= glm::translate(glm::mat4(1.0f), pair.first->position);
			glm::mat4 rotation		= glm::mat4_cast(pair.first->orientation);
			glm::mat4 scale			= glm::scale(glm::mat4(1.0f), pair.first->scale);
			pair.second->setTransforms(translation * rotation * scale);
		}

		// Detect the collisions between the colliders
		mCollisionWorld.update();

		// Notify the contact manifolds
		for (collision::Manifold* manifold : mCollisionWorld.getCollisionManifolds()) {
			auto itPair1 = mColliderEntityMap.find(manifold->colliders[0]);
			auto itPair2 = mColliderEntityMap.find(manifold->colliders[1]);
			if ((itPair1 != mColliderEntityMap.end()) && (itPair2 != mColliderEntityMap.end())) {
				auto event = new CollisionEvent(itPair1->second, itPair2->second, manifold);
				mEventManager.publish(event);
			}
		}

		SOMBRA_INFO_LOG << "CollisionManager updated";
	}

}
