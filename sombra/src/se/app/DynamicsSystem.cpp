#include "se/utils/Log.h"
#include "se/physics/RigidBody.h"
#include "se/physics/PhysicsEngine.h"
#include "se/app/DynamicsSystem.h"
#include "se/app/Application.h"
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
			},
			true
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
			},
			true
		);

		SOMBRA_INFO_LOG << "End";
	}

// Private functions
	void DynamicsSystem::onNewRigidBody(Entity entity, physics::RigidBody* rigidBody)
	{
		auto [transforms] = mEntityDatabase.getComponents<TransformsComponent>(entity, true);
		if (transforms) {
			transforms->updated.reset(static_cast<int>(TransformsComponent::Update::RigidBody));
		}

		mApplication.getExternalTools().physicsEngine->addRigidBody(rigidBody);
		SOMBRA_INFO_LOG << "Entity " << entity << " with RigidBody " << rigidBody << " added successfully";
	}


	void DynamicsSystem::onRemoveRigidBody(Entity entity, physics::RigidBody* rigidBody)
	{
		mApplication.getExternalTools().physicsEngine->removeRigidBody(rigidBody);
		SOMBRA_INFO_LOG << "Entity " << entity << " with RigidBody " << rigidBody << " removed successfully";
	}

}
