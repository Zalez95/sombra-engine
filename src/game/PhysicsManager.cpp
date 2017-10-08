#include "PhysicsManager.h"
#include "../physics/RigidBody.h"

namespace game {

	void PhysicsManager::addEntity(Entity* entity, PhysicsEntityUPtr physicsEntity, bool modifiable)
	{
		if (!entity || !physicsEntity) return;

		// The rigid body initial data is overrided by the entity one
		physics::RigidBody* rigidBody = physicsEntity->getRigidBody();
		rigidBody->mPosition			= entity->mPosition;
		rigidBody->mLinearVelocity		= entity->mVelocity;
		rigidBody->mOrientation			= entity->mOrientation;

		mPhysicsEngine.addPhysicsEntity(physicsEntity.get());
		mEntityMap.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(entity),
			std::forward_as_tuple(
				std::move(physicsEntity),
				modifiable
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
		// We update the rigid body data with the changes to the entity data
		for (auto& entry : mEntityMap) {
			if (entry.second.mModifiable) {
				Entity* entity = entry.first;
				physics::RigidBody* rigidBody = entry.second.mPhysicsEntity->getRigidBody();

				rigidBody->mPosition			= entity->mPosition;
				rigidBody->mLinearVelocity		= entity->mVelocity;
				rigidBody->mOrientation			= entity->mOrientation;
			}
		}

		// Update the rigid bodies
		mPhysicsEngine.update(delta);

		// We update the entity data with the changes to the rigid body data
		for (auto& entry : mEntityMap) {
			Entity* entity					= entry.first;
			physics::RigidBody* rigidBody	= entry.second.mPhysicsEntity->getRigidBody();

			entity->mPosition		= rigidBody->mPosition;
			entity->mVelocity		= rigidBody->mLinearVelocity;
			entity->mOrientation	= rigidBody->mOrientation;
		}
	}

}
