#include "PhysicsManager.h"
#include "../physics/RigidBody.h"

namespace game {

	void PhysicsManager::addEntity(Entity* entity, PhysicsEntityUPtr physicsEntity)
	{
		if (!entity || !physicsEntity) return;

		physics::RigidBody* rigidBody = physicsEntity->getRigidBody();
		rigidBody->mPosition	= entity->mPosition;
		rigidBody->mOrientation	= entity->mOrientation;

		mPhysicsEngine.addPhysicsEntity(physicsEntity.get());
		mEntityMap.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(entity),
			std::forward_as_tuple(
				std::move(physicsEntity),
				entity->mPosition,
				entity->mOrientation
			)
		);
	}


	void PhysicsManager::removeEntity(Entity* entity)
	{
		auto itEntity = mEntityMap.find(entity);
		if (itEntity != mEntityMap.end()) {
			mPhysicsEngine.removePhysicsEntity(itEntity->second.mPhysicsEntity.get());
			mEntityMap.erase(itEntity);
		}
	}


	void PhysicsManager::update(float delta)
	{
		mPhysicsEngine.update(delta);

		for (auto& entry : mEntityMap) {
			Entity* entity					= entry.first;
			PhysicsData& pData				= entry.second;
			physics::RigidBody* rigidBody	= pData.mPhysicsEntity->getRigidBody();

			// If the Entity position or orientation has been changed by other
			// component, update the rigid body data
			if (entity->mPosition != pData.mEntityLastPosition) {
				rigidBody->mPosition		+= entity->mPosition - pData.mEntityLastPosition;
			}
			if (entity->mOrientation != pData.mEntityLastOrientation) {
				glm::quat qDifference		= entity->mOrientation * glm::inverse(pData.mEntityLastOrientation);
				rigidBody->mOrientation		= qDifference * rigidBody->mOrientation;
			}

			// Update the Entity position and orientation with the changes to
			// the rigid body position and orientation
			entity->mPosition				= rigidBody->mPosition;
			entity->mOrientation			= rigidBody->mOrientation;

			// Update the entity last position and orientation
			pData.mEntityLastPosition		= entity->mPosition;
			pData.mEntityLastOrientation	= entity->mOrientation;
		}
	}

}
