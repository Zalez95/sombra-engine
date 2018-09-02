#include "fe/app/PhysicsManager.h"
#include "fe/physics/RigidBody.h"

namespace fe { namespace app {

	void PhysicsManager::addEntity(Entity* entity, PhysicsEntityUPtr physicsEntity)
	{
		if (!entity || !physicsEntity) return;

		// The rigid body initial data is overrided by the entity one
		physics::RigidBody* rigidBody = physicsEntity->getRigidBody();
		rigidBody->position			= entity->position;
		rigidBody->linearVelocity	= entity->velocity;
		rigidBody->orientation		= entity->orientation;
		physics::updateRigidBodyData(*rigidBody);

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
		// Update the RigidBodies with the changes made to the Entities
		for (auto& entry : mEntityMap) {
			Entity* entity = entry.first;
			physics::RigidBody* rigidBody = entry.second->getRigidBody();

			rigidBody->position			= entity->position;
			rigidBody->linearVelocity	= entity->velocity;
			rigidBody->orientation		= entity->orientation;
			physics::updateRigidBodyData(*rigidBody);
		}

		// Update the RigidBodies
		mPhysicsEngine.update(delta);

		// Update the Entities with the changes made to the RigidBodies
		for (auto& entry : mEntityMap) {
			Entity* entity					= entry.first;
			physics::RigidBody* rigidBody	= entry.second->getRigidBody();

			entity->position	= rigidBody->position;
			entity->velocity	= rigidBody->linearVelocity;
			entity->orientation	= rigidBody->orientation;
		}
	}

}}
