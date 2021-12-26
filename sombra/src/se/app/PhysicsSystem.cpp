#include "se/utils/Log.h"
#include "se/physics/RigidBodyWorld.h"
#include "se/app/TransformsComponent.h"
#include "se/app/RigidBodyComponent.h"
#include "se/app/Application.h"
#include "se/app/PhysicsSystem.h"

namespace se::app {

	PhysicsSystem::PhysicsSystem(Application& application) :
		ISystem(application.getEntityDatabase()), mApplication(application)
	{
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask().set<RigidBodyComponent>());
	}


	PhysicsSystem::~PhysicsSystem()
	{
		mEntityDatabase.removeSystem(this);
	}


	void PhysicsSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Start";

		SOMBRA_DEBUG_LOG << "Updating the RigidBodies";
		mEntityDatabase.executeQuery([this](EntityDatabase::Query& query) {
			query.iterateEntityComponents<TransformsComponent, RigidBodyComponent>(
				[this](Entity, TransformsComponent* transforms, RigidBodyComponent* rigidBody) {
					if (!transforms->updated[static_cast<int>(TransformsComponent::Update::RigidBody)]) {
						physics::RigidBodyState state = rigidBody->get().getState();
						state.position			= transforms->position;
						state.linearVelocity	= transforms->velocity;
						state.orientation		= transforms->orientation;
						rigidBody->get().setState(state);

						transforms->updated.set(static_cast<int>(TransformsComponent::Update::RigidBody));
					}
				},
				true
			);
		});

		SOMBRA_DEBUG_LOG << "Updating the RigidBodyWorld";
		mApplication.getExternalTools().rigidBodyWorld->update(mDeltaTime);

		SOMBRA_DEBUG_LOG << "Updating the Entities";
		mEntityDatabase.executeQuery([this](EntityDatabase::Query& query) {
			query.iterateEntityComponents<TransformsComponent, RigidBodyComponent>(
				[this](Entity, TransformsComponent* transforms, RigidBodyComponent* rigidBody) {
					if (!rigidBody->get().getStatus(physics::RigidBody::Status::Sleeping)) {
						transforms->position	= rigidBody->get().getState().position;
						transforms->velocity	= rigidBody->get().getState().linearVelocity;
						transforms->orientation	= rigidBody->get().getState().orientation;

						transforms->updated.reset().set(static_cast<int>(TransformsComponent::Update::RigidBody));
					}
				},
				true
			);
		});

		SOMBRA_DEBUG_LOG << "End";
	}

// Private functions
	void PhysicsSystem::onNewRigidBody(Entity entity, RigidBodyComponent* rigidBody, EntityDatabase::Query& query)
	{
		auto [transforms] = query.getComponents<TransformsComponent>(entity, true);
		if (transforms) {
			transforms->updated.reset(static_cast<int>(TransformsComponent::Update::RigidBody));
		}

		auto properties = rigidBody->get().getProperties();
		uintptr_t userData = entity;
		properties.userData = reinterpret_cast<void*>(userData);
		rigidBody->get().setProperties(properties);

		mApplication.getExternalTools().rigidBodyWorld->addRigidBody(&rigidBody->get());
		SOMBRA_INFO_LOG << "Entity " << entity << " with RigidBodyComponent " << rigidBody << " added successfully";
	}


	void PhysicsSystem::onRemoveRigidBody(Entity entity, RigidBodyComponent* rigidBody, EntityDatabase::Query&)
	{
		mApplication.getExternalTools().rigidBodyWorld->removeRigidBody(&rigidBody->get());
		SOMBRA_INFO_LOG << "Entity " << entity << " with RigidBodyComponent " << rigidBody << " removed successfully";
	}

}
