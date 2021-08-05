#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include "se/utils/Log.h"
#include "se/collision/Collider.h"
#include "se/collision/CollisionWorld.h"
#include "se/app/CollisionSystem.h"
#include "se/app/Application.h"
#include "se/app/TransformsComponent.h"
#include "se/app/events/CollisionEvent.h"

namespace se::app {

	CollisionSystem::CollisionSystem(Application& application) :
		ISystem(application.getEntityDatabase()), mApplication(application)
	{
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask().set<collision::Collider>());
	}


	CollisionSystem::~CollisionSystem()
	{
		mEntityDatabase.removeSystem(this);
	}


	void CollisionSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the CollisionSystem";
		auto& collisionWorld = *mApplication.getExternalTools().collisionWorld;

		SOMBRA_DEBUG_LOG << "Updating Colliders";
		mEntityDatabase.iterateComponents<TransformsComponent, collision::Collider>(
			[this](Entity, TransformsComponent* transforms, collision::Collider* collider) {
				if (!transforms->updated[static_cast<int>(TransformsComponent::Update::Collider)]) {
					collider->setTransforms(getModelMatrix(*transforms));
					transforms->updated.set(static_cast<int>(TransformsComponent::Update::Collider));
				}
			},
			true
		);

		SOMBRA_DEBUG_LOG << "Detecting collisions between the colliders";
		collisionWorld.update();

		SOMBRA_DEBUG_LOG << "Notifying contact manifolds";
		collisionWorld.processCollisionManifolds([this](const collision::Manifold& manifold) {
			auto entity1 = mEntityDatabase.getEntity(manifold.colliders[0]);
			auto entity2 = mEntityDatabase.getEntity(manifold.colliders[1]);
			if ((entity1 != kNullEntity) && (entity2 != kNullEntity)
				&& manifold.state[collision::Manifold::State::Updated]
			) {
				auto event = new CollisionEvent(entity1, entity2, &manifold);
				SOMBRA_DEBUG_LOG << "Notifying new CollisionEvent " << *event;
				mApplication.getEventManager().publish(event);
			}
		});

		SOMBRA_DEBUG_LOG << "CollisionSystem updated";
	}


	std::vector<CollisionSystem::EntityRayCastPair> CollisionSystem::getEntities(
		const glm::vec3& rayOrigin, const glm::vec3& rayDirection
	) const
	{
		SOMBRA_INFO_LOG << "Performing rayCast from "
			<< glm::to_string(rayOrigin) << " towards " << glm::to_string(rayDirection);

		std::vector<EntityRayCastPair> ret;

		mApplication.getExternalTools().collisionWorld->processRayCast(
			rayOrigin, rayDirection,
			[&](const collision::Collider& collider, const collision::RayCast& rayCast) {
				auto entity = mEntityDatabase.getEntity(&collider);
				if (entity != kNullEntity) {
					SOMBRA_DEBUG_LOG << "RayCast against Entity " << entity << " OK";
					ret.emplace_back(entity, rayCast);
				}
			}
		);

		SOMBRA_INFO_LOG << "RayCast finished with " << ret.size() << " entities";
		return ret;
	}

// Private functions
	void CollisionSystem::onNewCollider(Entity entity, collision::Collider* collider)
	{
		auto [transforms] = mEntityDatabase.getComponents<TransformsComponent>(entity, true);
		if (transforms) {
			transforms->updated.reset(static_cast<int>(TransformsComponent::Update::Collider));
		}

		mApplication.getExternalTools().collisionWorld->addCollider(collider);
		SOMBRA_INFO_LOG << "Entity " << entity << " with Collider " << collider << " added successfully";
	}


	void CollisionSystem::onRemoveCollider(Entity entity, collision::Collider* collider)
	{
		mApplication.getExternalTools().collisionWorld->removeCollider(collider);
		SOMBRA_INFO_LOG << "Entity " << entity << " with Collider " << collider << " removed successfully";
	}

}
