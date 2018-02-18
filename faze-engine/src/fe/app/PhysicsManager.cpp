#include "fe/app/PhysicsManager.h"
#include "fe/physics/RigidBody.h"

namespace fe { namespace app {

	void PhysicsManager::addEntity(Entity* entity, PhysicsEntityUPtr physicsEntity)
	{
		if (!entity || !physicsEntity) return;

		// The rigid body initial data is overrided by the entity one
		physics::RigidBody* rigidBody = physicsEntity->getRigidBody();
		rigidBody->setPosition(entity->mPosition);
		rigidBody->setLinearVelocity(entity->mVelocity);
		rigidBody->setOrientation(entity->mOrientation);
		rigidBody->updateData();

		mPhysicsEngine.addPhysicsEntity(physicsEntity.get());
		mEntityMap[entity] = std::move(physicsEntity);
	}


	void PhysicsManager::removeEntity(Entity* entity)
	{
		auto itEntity = mEntityMap.find(entity);
		if (itEntity != mEntityMap.end()) {
			mPhysicsEngine.removePhysicsEntity(itEntity->second.get());
			mEntityMap.erase(itEntity);
		}
	}


	void PhysicsManager::update(float delta)
	{
		// We update the rigid body data with the changes to the entity data
		for (auto& entry : mEntityMap) {
			Entity* entity = entry.first;
			physics::RigidBody* rigidBody = entry.second->getRigidBody();

			rigidBody->setPosition(entity->mPosition);
			rigidBody->setLinearVelocity(entity->mVelocity);
			rigidBody->setOrientation(entity->mOrientation);
			rigidBody->updateData();
		}

		// Update the rigid bodies
		mPhysicsEngine.update(delta);

		// We update the entity data with the changes to the rigid body data
		for (auto& entry : mEntityMap) {
			Entity* entity					= entry.first;
			physics::RigidBody* rigidBody	= entry.second->getRigidBody();

			entity->mPosition		= rigidBody->getPosition();
			entity->mVelocity		= rigidBody->getLinearVelocity();
			entity->mOrientation	= rigidBody->getOrientation();
		}
	}

}}
