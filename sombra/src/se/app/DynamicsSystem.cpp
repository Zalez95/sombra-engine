#include "se/utils/Log.h"
#include "se/physics/RigidBody.h"
#include "se/physics/PhysicsEngine.h"
#include "se/app/DynamicsSystem.h"
#include "se/app/Application.h"
#include "se/app/EntityDatabase.h"
#include "se/app/TransformsComponent.h"

namespace se::app {

	DynamicsSystem::DynamicsSystem(Application& application) :
		ISystem(application.getEntityDatabase()), mApplication(application)
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
			transforms->updated.reset(static_cast<int>(TransformsComponent::Update::RigidBody));
		}

		mApplication.getExternalTools().physicsEngine->addRigidBody(rb);
		SOMBRA_INFO_LOG << "Entity " << entity << " with RigidBody " << rb << " added successfully";
	}


	void DynamicsSystem::onRemoveEntity(Entity entity)
	{
		auto [rb] = mEntityDatabase.getComponents<physics::RigidBody>(entity);
		if (!rb) {
			SOMBRA_WARN_LOG << "Entity " << entity << " wasn't removed";
			return;
		}

		mApplication.getExternalTools().physicsEngine->removeRigidBody(rb);
		SOMBRA_INFO_LOG << "Entity " << entity << " removed successfully";
	}


	void DynamicsSystem::update()
	{
		SOMBRA_INFO_LOG << "Start";

		mApplication.getExternalTools().physicsEngine->resetRigidBodiesState();

		SOMBRA_DEBUG_LOG << "Updating the RigidBodies";
		mEntityDatabase.iterateComponents<TransformsComponent, physics::RigidBody>(
			[this](Entity, TransformsComponent* transforms, physics::RigidBody* rigidBody) {
				if (!transforms->updated[static_cast<int>(TransformsComponent::Update::RigidBody)]) {
					rigidBody->getData().position		= transforms->position;
					rigidBody->getData().linearVelocity	= transforms->velocity;
					rigidBody->getData().orientation	= transforms->orientation;
					rigidBody->synchWithData();

					transforms->updated.set(static_cast<int>(TransformsComponent::Update::RigidBody));
				}
			}
		);

		SOMBRA_DEBUG_LOG << "Integrating the RigidBodies";
		mApplication.getExternalTools().physicsEngine->integrate(mDeltaTime);

		SOMBRA_DEBUG_LOG << "Updating the Entities";
		mEntityDatabase.iterateComponents<TransformsComponent, physics::RigidBody>(
			[this](Entity, TransformsComponent* transforms, physics::RigidBody* rigidBody) {
				if (rigidBody->checkState(physics::RigidBodyState::Integrated)) {
					transforms->position	= rigidBody->getData().position;
					transforms->velocity	= rigidBody->getData().linearVelocity;
					transforms->orientation	= rigidBody->getData().orientation;

					transforms->updated.reset().set(static_cast<int>(TransformsComponent::Update::RigidBody));
				}
			}
		);

		SOMBRA_INFO_LOG << "End";
	}

}
