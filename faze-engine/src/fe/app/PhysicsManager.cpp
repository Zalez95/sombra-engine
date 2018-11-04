#include "fe/app/Entity.h"
#include "fe/app/PhysicsManager.h"
#include "fe/physics/RigidBody.h"

namespace fe { namespace app {

	void PhysicsManager::addEntity(Entity* entity, RigidBodyUPtr rigidBody)
	{
		if (!entity || !rigidBody) return;

		// The rigid body initial data is overrided by the entity one
		rigidBody->position			= entity->position;
		rigidBody->linearVelocity	= entity->velocity;
		rigidBody->orientation		= entity->orientation;
		physics::updateRigidBodyData(*rigidBody);

		mPhysicsEngine.addRigidBody(rigidBody.get());
		mEntityMap[entity] = std::move(rigidBody);
	}


	void PhysicsManager::removeEntity(Entity* entity)
	{
		auto itEntity = mEntityMap.find(entity);
		if (itEntity != mEntityMap.end()) {
			mPhysicsEngine.removeRigidBody(itEntity->second.get());
			mEntityMap.erase(itEntity);
		}
	}


	void PhysicsManager::doDynamics(float delta)
	{
		// Update the RigidBodies with the changes made to the Entities
		for (auto& pair : mEntityMap) {
			Entity* entity = pair.first;
			physics::RigidBody* rigidBody = pair.second.get();

			rigidBody->position			= entity->position;
			rigidBody->linearVelocity	= entity->velocity;
			rigidBody->orientation		= entity->orientation;
			physics::updateRigidBodyData(*rigidBody);
		}

		mPhysicsEngine.integrate(delta);

		// Update the Entities with the changes made to the RigidBodies
		for (auto& pair : mEntityMap) {
			Entity* entity					= pair.first;
			physics::RigidBody* rigidBody	= pair.second.get();

			entity->position	= rigidBody->position;
			entity->velocity	= rigidBody->linearVelocity;
			entity->orientation	= rigidBody->orientation;
		}
	}


	void PhysicsManager::doConstraints(float delta)
	{
		// Update the RigidBodies with the changes made to the Entities
		for (auto& pair : mEntityMap) {
			Entity* entity = pair.first;
			physics::RigidBody* rigidBody = pair.second.get();

			rigidBody->position			= entity->position;
			rigidBody->linearVelocity	= entity->velocity;
			rigidBody->orientation		= entity->orientation;
			physics::updateRigidBodyData(*rigidBody);
		}

		mPhysicsEngine.solveConstraints(delta);

		// Update the Entities with the changes made to the RigidBodies
		for (auto& pair : mEntityMap) {
			Entity* entity					= pair.first;
			physics::RigidBody* rigidBody	= pair.second.get();

			entity->position	= rigidBody->position;
			entity->velocity	= rigidBody->linearVelocity;
			entity->orientation	= rigidBody->orientation;
		}
	}

}}
