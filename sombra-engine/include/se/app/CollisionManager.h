#ifndef COLLISION_MANAGER_H
#define COLLISION_MANAGER_H

#include <map>
#include <memory>
#include "../collision/Collider.h"
#include "../collision/CollisionWorld.h"
#include "events/EventManager.h"

namespace se::app {

	struct Entity;


	/**
	 * Class CollisionManager, it's a Manager used for storing and updating
	 * the Entities' collision data
	 */
	class CollisionManager
	{
	private:	// Nested types
		using ColliderUPtr = std::unique_ptr<collision::Collider>;
		using EntityRayCastPair = std::pair<Entity*, collision::RayCast>;

	private:	// Attributes
		/** The CollisionWorld used for detecting the collisions the data of
		 * the Colliders */
		collision::CollisionWorld& mCollisionWorld;

		/** The EventManager used for notifying events */
		EventManager& mEventManager;

		/** Maps the Entities added to the CollisionManager and its Colliders */
		std::map<Entity*, ColliderUPtr> mEntityColliderMap;

		/** Maps the Colliders added to the CollisionManager and its Entities */
		std::map<const collision::Collider*, Entity*> mColliderEntityMap;

	public:		// Functions
		/** Creates a new CollisionManager
		 *
		 * @param	collisionWorld a reference to the CollisionWorld used
		 *			by the CollisionManager
		 * @param	eventManager a reference to the EventManager that the
		 *			CollisionManager will use to notify the detected
		 *			collisions */
		CollisionManager(
			collision::CollisionWorld& collisionWorld,
			EventManager& eventManager
		);

		/** Adds the given Entity to the CollisionManager and its collider data
		 * to the manager
		 *
		 * @param	entity a pointer to the Entity to add to the
		 *			CollisionManager
		 * @param	collider the collider data of the Entity
		 * @note	The Collider initial data is overridden by the Entity one */
		void addEntity(Entity* entity, ColliderUPtr collider);

		/** Removes the given Entity from the CollisionManager so it won't
		 * longer be updated
		 *
		 * @param	entity a pointer to the Entity to remove from the
		 *			CollisionManager */
		void removeEntity(Entity* entity);

		/** Detects the collisions between the entities' colliders
		 *
		 * @param	delta the elapsed time since the last update in seconds */
		void update(float delta);

		std::vector<EntityRayCastPair> getEntities(
			const glm::vec3& rayOrigin, const glm::vec3& rayDirection
		) const;
	};

}

#endif	// COLLISION_MANAGER_H
