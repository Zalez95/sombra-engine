#ifndef COLLISION_SYSTEM_H
#define COLLISION_SYSTEM_H

#include "../collision/RayCast.h"
#include "ECS.h"

namespace se::collision { class Collider; }

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

		/** @copydoc ISystem::onNewComponent(Entity, const EntityDatabase::ComponentMask&) */
		virtual void onNewComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask
		) override
		{ tryCallC(&CollisionSystem::onNewCollider, entity, mask); };

		/** @copydoc ISystem::onRemoveComponent(Entity, const EntityDatabase::ComponentMask&) */
		virtual void onRemoveComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask
		) override
		{ tryCallC(&CollisionSystem::onRemoveCollider, entity, mask); };

		/** Detects the collisions between the entities' colliders */
		virtual void update() override;

		/** TODO: */
		std::vector<EntityRayCastPair> getEntities(
			const glm::vec3& rayOrigin, const glm::vec3& rayDirection
		) const;
	private:
		/** Function called when a Collider is added to an Entity
		 *
		 * @param	entity the Entity that holds the Collider
		 * @param	collider a pointer to the new Collider */
		void onNewCollider(Entity entity, collision::Collider* collider);

		/** Function called when a Collider is going to be removed from an
		 * Entity
		 *
		 * @param	entity the Entity that holds the Collider
		 * @param	collider a pointer to the Collider that is going to be
		 *			removed */
		void onRemoveCollider(Entity entity, collision::Collider* collider);
	};

}

#endif	// COLLISION_SYSTEM_H
