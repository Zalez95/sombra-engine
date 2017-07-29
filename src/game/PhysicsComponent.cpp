#include "PhysicsComponent.h"
#include "Entity.h"

namespace game {

	PhysicsComponent::PhysicsComponent(
		physics::PhysicsEngine& physicsEngine,
		std::unique_ptr<physics::PhysicsEntity> physicsEntity
	) : mPhysicsEngine(physicsEngine), mPhysicsEntity(std::move(physicsEntity))
	{
		physics::RigidBody* rigidBody = mPhysicsEntity->getRigidBody();
		mLastPosition = rigidBody->mPosition;
		mLastOrientation = rigidBody->mOrientation;

		mPhysicsEngine.addPhysicsEntity(mPhysicsEntity.get());
	}


	PhysicsComponent::~PhysicsComponent()
	{
		mPhysicsEngine.removePhysicsEntity(mPhysicsEntity.get());
	}


	void PhysicsComponent::update(Entity& entity, float delta)
	{
		physics::RigidBody* rigidBody = mPhysicsEntity->getRigidBody();
		
		// If the Entity position or orientation has been changed by other
		// component, update the rigid body data
		if (entity.mPosition != mLastPosition) {
			rigidBody->mPosition += entity.mPosition - mLastPosition;
		}
		if (entity.mOrientation != mLastOrientation) {
			rigidBody->mOrientation *= mLastOrientation * glm::inverse(entity.mOrientation);
		}

		// Update the entity with the changes to the rigid body position and
		// orientation
		entity.mPosition = rigidBody->mPosition;
		entity.mOrientation = rigidBody->mOrientation;

		mLastPosition = rigidBody->mPosition;
		mLastOrientation = rigidBody->mOrientation;
	}

}
