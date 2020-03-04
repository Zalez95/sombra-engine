#include <glm/gtc/matrix_transform.hpp>
#include "se/utils/Log.h"
#include "se/app/Entity.h"
#include "se/app/CollisionManager.h"
#include "se/app/events/CollisionEvent.h"
#include "se/collision/ConvexCollider.h"

namespace se::app {

	CollisionManager::CollisionManager(collision::CollisionWorld& collisionWorld, EventManager& eventManager) :
		mCollisionWorld(collisionWorld), mEventManager(eventManager) {}


	void CollisionManager::addEntity(Entity* entity, ColliderUPtr collider)
	{
		if (!entity || !collider) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added";
			return;
		}

		// The Collider initial data is overridden by the entity one
		collision::Collider* cPtr = collider.get();
		glm::mat4 translation	= glm::translate(glm::mat4(1.0f), entity->position);
		glm::mat4 rotation		= glm::mat4_cast(entity->orientation);
		glm::mat4 scale			= glm::scale(glm::mat4(1.0f), entity->scale);
		cPtr->setTransforms(translation * rotation * scale);

		// Add the Collider
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

		SOMBRA_DEBUG_LOG << "Updating Colliders";
		for (auto& pair : mEntityColliderMap) {
			Entity* entity = pair.first;
			collision::Collider* collider = pair.second.get();

			if (entity->updated.any()) {
				glm::mat4 translation	= glm::translate(glm::mat4(1.0f), entity->position);
				glm::mat4 rotation		= glm::mat4_cast(entity->orientation);
				glm::mat4 scale			= glm::scale(glm::mat4(1.0f), entity->scale);
				collider->setTransforms(translation * rotation * scale);
			}
		}

		SOMBRA_DEBUG_LOG << "Detecting collisions between the colliders";
		mCollisionWorld.update();

		SOMBRA_DEBUG_LOG << "Notifying contact manifolds";
		for (const collision::Manifold* manifold : mCollisionWorld.getCollisionManifolds()) {
			auto itPair1 = mColliderEntityMap.find(manifold->colliders[0]);
			auto itPair2 = mColliderEntityMap.find(manifold->colliders[1]);
			if ((itPair1 != mColliderEntityMap.end()) && (itPair2 != mColliderEntityMap.end())
				&& manifold->state[collision::Manifold::State::Updated]
			) {
				auto event = new CollisionEvent(itPair1->second, itPair2->second, manifold);
				SOMBRA_DEBUG_LOG << "Notifying new CollisionEvent " << *event;
				mEventManager.publish(event);
			}
		}

		SOMBRA_INFO_LOG << "CollisionManager updated";
	}


	std::string CollisionManager::getName(const glm::vec3& rayOrigin, const glm::vec3& rayDirection)
	{
		collision::GJKCollisionDetector gjk(0.0001f);
		for (auto& pair : mColliderEntityMap) {
			if (auto collider = dynamic_cast<const collision::ConvexCollider*>(pair.first)) {
				if (gjk.calculateRayCast(rayOrigin, rayDirection, *collider)) {
					return pair.second->name;
				}
			}
		}
		return "";
	}

}
