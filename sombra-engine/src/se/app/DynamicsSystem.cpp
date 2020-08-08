#include "se/utils/Log.h"
#include "se/physics/RigidBody.h"
#include "se/app/DynamicsSystem.h"
#include "se/app/EntityDatabase.h"
#include "se/app/TransformsComponent.h"

namespace se::app {

	DynamicsSystem::DynamicsSystem(EntityDatabase& entityDatabase, physics::PhysicsEngine& physicsEngine) :
		ISystem(entityDatabase), mPhysicsEngine(physicsEngine)
	{
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask().set<physics::RigidBody>());
	}


	DynamicsSystem::~DynamicsSystem()
	{
		mEntityDatabase.removeSystem(this);
	}


	void DynamicsSystem::onNewEntity(Entity entity)
	{
		auto [transforms, rb] = mEntityDatabase.getComponents<TransformsComponent, physics::RigidBody>(entity);
		if (!rb) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as RigidBody";
			return;
		}

		if (transforms) {
			// The RigidBody initial data is overridden by the entity one
			rb->getData().position			= transforms->position;
			rb->getData().linearVelocity	= transforms->velocity;
			rb->getData().orientation		= transforms->orientation;
			rb->synchWithData();
		}

		mPhysicsEngine.addRigidBody(rb);
		SOMBRA_INFO_LOG << "Entity " << entity << " with RigidBody " << rb << " added successfully";
	}


	void DynamicsSystem::onRemoveEntity(Entity entity)
	{
		auto [rb] = mEntityDatabase.getComponents<physics::RigidBody>(entity);
		if (!rb) {
			SOMBRA_WARN_LOG << "Entity " << entity << " wasn't removed";
			return;
		}

		mPhysicsEngine.removeRigidBody(rb);
		SOMBRA_INFO_LOG << "Entity " << entity << " removed successfully";
	}


	void DynamicsSystem::update()
	{
		SOMBRA_INFO_LOG << "Start";

		mPhysicsEngine.resetRigidBodiesState();

		SOMBRA_DEBUG_LOG << "Updating the RigidBodies";
		mEntityDatabase.iterateComponents<TransformsComponent, physics::RigidBody>(
			[this](Entity, TransformsComponent* transforms, physics::RigidBody* rigidBody) {
				transforms->updated.reset( static_cast<int>(TransformsComponent::Update::Physics) );
				if (transforms->updated.any()) {
					rigidBody->getData().position		= transforms->position;
					rigidBody->getData().linearVelocity	= transforms->velocity;
					rigidBody->getData().orientation	= transforms->orientation;
					rigidBody->synchWithData();
				}
			}
		);

		SOMBRA_DEBUG_LOG << "Integrating the RigidBodies";
		mPhysicsEngine.integrate(mDeltaTime);

		SOMBRA_DEBUG_LOG << "Updating the Entities";
		mEntityDatabase.iterateComponents<TransformsComponent, physics::RigidBody>(
			[this](Entity, TransformsComponent* transforms, physics::RigidBody* rigidBody) {
				if (rigidBody->checkState(physics::RigidBodyState::Integrated)) {
					transforms->position	= rigidBody->getData().position;
					transforms->velocity	= rigidBody->getData().linearVelocity;
					transforms->orientation	= rigidBody->getData().orientation;
					transforms->updated.set( static_cast<int>(TransformsComponent::Update::Physics) );
				}
			}
		);

		SOMBRA_INFO_LOG << "End";
	}

}
