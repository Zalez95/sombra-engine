#ifndef COLLISION_SYSTEM_H
#define COLLISION_SYSTEM_H

#include "../collision/CollisionWorld.h"
#include "ISystem.h"

namespace se::app {

	/**
	 * Class CollisionSystem, it's a System used for updating the Entities'
	 * collision data
	 */
	class CollisionSystem : public ISystem
	{
	private:	// Nested types
		using EntityRayCastPair = std::pair<Entity, collision::RayCast>;

	private:	// Attributes
		/** The EventManager used for notifying events */
		EventManager& mEventManager;

		/** The CollisionWorld used for detecting the collisions the data of
		 * the Colliders */
		collision::CollisionWorld& mCollisionWorld;

	public:		// Functions
		/** Creates a new CollisionSystem
		 *
		 * @param	entityDatabase the EntityDatabase that holds all the
		 *			Entities
		 * @param	eventManager a reference to the EventManager that the
		 *			CollisionSystem will use to notify the detected
		 *			collisions
		 * @param	collisionWorld a reference to the CollisionWorld used
		 *			by the CollisionSystem */
		CollisionSystem(
			EntityDatabase& entityDatabase, EventManager& eventManager,
			collision::CollisionWorld& collisionWorld
		);

		/** Class destructor */
		~CollisionSystem();

		/** Function that the EntityDatabase will call when an Entity is
		 * added
		 *
		 * @param	entity the new Entity */
		virtual void onNewEntity(Entity entity);

		/** Function that the EntityDatabase will call when an Entity is
		 * removed
		 *
		 * @param	entity the Entity to remove */
		virtual void onRemoveEntity(Entity entity);

		/** Detects the collisions between the entities' colliders */
		virtual void update() override;

		/** TODO: */
		std::vector<EntityRayCastPair> getEntities(
			const glm::vec3& rayOrigin, const glm::vec3& rayDirection
		) const;
	};

}

#endif	// COLLISION_SYSTEM_H
