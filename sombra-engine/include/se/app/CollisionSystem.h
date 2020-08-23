#ifndef COLLISION_SYSTEM_H
#define COLLISION_SYSTEM_H

#include "../collision/RayCast.h"
#include "ISystem.h"

namespace se::app {

	class Application;


	/**
	 * Class CollisionSystem, it's a System used for updating the Entities'
	 * collision data
	 */
	class CollisionSystem : public ISystem
	{
	private:	// Nested types
		using EntityRayCastPair = std::pair<Entity, collision::RayCast>;

	private:	// Attributes
		/** The Application that holds the CollisionWorld and the EventManager
		 * used for detecting the collisions between the Entities' Colliders
		 * and notifying them */
		Application& mApplication;

	public:		// Functions
		/** Creates a new CollisionSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System */
		CollisionSystem(Application& application);

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
