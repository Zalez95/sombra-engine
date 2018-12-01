#include "se/utils/Log.h"
#include "se/app/Entity.h"
#include "se/app/PhysicsManager.h"
#include "se/physics/RigidBody.h"

namespace se::app {

	void PhysicsManager::addEntity(Entity* entity, RigidBodyUPtr rigidBody)
	{
		if (!entity || !rigidBody) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added";
			return;
		}

		// The rigid body initial data is overrided by the entity one
		rigidBody->position			= entity->position;
		rigidBody->linearVelocity	= entity->velocity;
		rigidBody->orientation		= entity->orientation;
		physics::updateRigidBodyData(*rigidBody);

		mPhysicsEngine.addRigidBody(rigidBody.get());
		mEntityMap.emplace(entity, std::move(rigidBody));
		SOMBRA_INFO_LOG << "Entity " << entity << " added successfully";
	}


	void PhysicsManager::removeEntity(Entity* entity)
	{
		auto itEntity = mEntityMap.find(entity);
		if (itEntity != mEntityMap.end()) {
			mPhysicsEngine.removeRigidBody(itEntity->second.get());
			mEntityMap.erase(itEntity);
			SOMBRA_INFO_LOG << "Entity " << entity << " removed successfully";
		}
		else {
			SOMBRA_WARN_LOG << "Entity " << entity << " wasn't removed";
		}
	}


	void PhysicsManager::doDynamics(float delta)
	{
		SOMBRA_INFO_LOG << "Start (" << delta << ")";

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

		SOMBRA_INFO_LOG << "End (" << delta << ")";
	}


	void PhysicsManager::doConstraints(float delta)
	{
		SOMBRA_INFO_LOG << "Start (" << delta << ")";

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

		SOMBRA_INFO_LOG << "End";
	}

}
