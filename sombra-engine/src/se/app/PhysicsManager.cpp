#include "se/utils/Log.h"
#include "se/app/Entity.h"
#include "se/app/PhysicsManager.h"
#include "se/app/events/CollisionEvent.h"
#include "se/physics/RigidBody.h"

namespace se::app {

	PhysicsManager::PhysicsManager(physics::PhysicsEngine& physicsEngine, EventManager& eventManager) :
		mPhysicsEngine(physicsEngine), mEventManager(eventManager)
	{
		mEventManager.subscribe(this, Topic::Collision);
	}


	PhysicsManager::~PhysicsManager()
	{
		mEventManager.unsubscribe(this, Topic::Collision);
	}


	void PhysicsManager::notify(const IEvent& event)
	{
		tryCall(&PhysicsManager::onCollisionEvent, event);
	}


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

		physics::RigidBody* rbPtr = rigidBody.get();
		mPhysicsEngine.addRigidBody(rbPtr);
		mEntityRBMap.emplace(entity, std::move(rigidBody));
		SOMBRA_INFO_LOG << "Entity " << entity << " with RigidBody " << rbPtr << " added successfully";
	}


	void PhysicsManager::removeEntity(Entity* entity)
	{
		auto itEntity = mEntityRBMap.find(entity);
		if (itEntity != mEntityRBMap.end()) {
			mPhysicsEngine.removeRigidBody(itEntity->second.get());
			mEntityRBMap.erase(itEntity);
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
		for (auto& pair : mEntityRBMap) {
			Entity* entity = pair.first;
			physics::RigidBody* rigidBody = pair.second.get();

			rigidBody->position			= entity->position;
			rigidBody->linearVelocity	= entity->velocity;
			rigidBody->orientation		= entity->orientation;
			physics::updateRigidBodyData(*rigidBody);
		}

		mPhysicsEngine.integrate(delta);

		// Update the Entities with the changes made to the RigidBodies
		for (auto& pair : mEntityRBMap) {
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
		for (auto& pair : mEntityRBMap) {
			Entity* entity = pair.first;
			physics::RigidBody* rigidBody = pair.second.get();

			rigidBody->position			= entity->position;
			rigidBody->linearVelocity	= entity->velocity;
			rigidBody->orientation		= entity->orientation;
			physics::updateRigidBodyData(*rigidBody);
		}

		// Update the NormalConstraints time
		for (auto& pair : mManifoldConstraints) {
			for (physics::NormalConstraint& constraint : pair.second) {
				constraint.setDeltaTime(delta);
				constraint.increaseK();
			}
		}

		mPhysicsEngine.solveConstraints(delta);

		// Update the Entities with the changes made to the RigidBodies
		for (auto& pair : mEntityRBMap) {
			Entity* entity = pair.first;
			physics::RigidBody* rigidBody = pair.second.get();

			entity->position	= rigidBody->position;
			entity->velocity	= rigidBody->linearVelocity;
			entity->orientation	= rigidBody->orientation;
		}

		SOMBRA_INFO_LOG << "End";
	}

// Private functions
	void PhysicsManager::onCollisionEvent(const CollisionEvent& event)
	{
		Entity* entity1 = event.getEntity(0);
		Entity* entity2 = event.getEntity(1);
		const collision::Manifold* manifold = event.getManifold();

		auto itEntityRB1 = mEntityRBMap.find(entity1);
		auto itEntityRB2 = mEntityRBMap.find(entity2);

		if ((itEntityRB1 != mEntityRBMap.end()) && (itEntityRB2 != mEntityRBMap.end()) && manifold) {
			physics::RigidBody* rb1 = itEntityRB1->second.get();
			physics::RigidBody* rb2 = itEntityRB2->second.get();

			switch (manifold->state) {
				case collision::ManifoldState::Intersecting:
					handleIntersectingManifold(rb1, rb2, manifold);
					break;
				case collision::ManifoldState::Disjoint:
					handleDisjointManifold(rb1, rb2, manifold);
					break;
			}
		}
		else {
			SOMBRA_ERROR_LOG << "Wrong CollisionEvent data: " << event;
		}
	}


	void PhysicsManager::handleIntersectingManifold(
		physics::RigidBody* rb1, physics::RigidBody* rb2,
		const collision::Manifold* manifold
	) {
		auto itPair = mManifoldConstraints.find(manifold);
		if (itPair == mManifoldConstraints.end()) {
			// Create the new manifold-constraints pair
			itPair = mManifoldConstraints.emplace(
				std::piecewise_construct,
				std::forward_as_tuple(manifold),
				std::forward_as_tuple()
			).first;
		}

		if (itPair->first->contacts.size() > itPair->second.size()) {
			// Increase the number of constraints up to the number of contacts
			for (std::size_t i = 0; i < itPair->first->contacts.size() - itPair->second.size(); ++i) {
				auto itConstraint = itPair->second.emplace_back(
					std::array<physics::RigidBody*, 2>{ rb1, rb2 },
					kCollisionConstraintBeta
				);
				mPhysicsEngine.getConstraintManager().addConstraint(&(*itConstraint));
				SOMBRA_DEBUG_LOG << "Added NormalConstraint[" << itPair->second.size() - 1 << "] between "
					<< rb1 << " and " << rb2;
			}
		}
		else if (itPair->first->contacts.size() < itPair->second.size()) {
			// Reduce the number of constraints up to the number of contacts
			for (std::size_t i = 0; i < itPair->second.size() - itPair->first->contacts.size(); ++i) {
				mPhysicsEngine.getConstraintManager().removeConstraint(&itPair->second.back());
				itPair->second.pop_back();
				SOMBRA_DEBUG_LOG << "Removed NormalConstraint[" << itPair->second.size() - 1 << "] between "
					<< rb1 << " and " << rb2;
			}
		}

		// Update the constraints data
		for (std::size_t i = 0; i < itPair->second.size(); ++i) {
			itPair->second[i].setConstraintNormal(itPair->first->contacts[i].normal);
			itPair->second[i].setConstraintPoints({
				itPair->first->contacts[i].localPosition[0],
				itPair->first->contacts[i].localPosition[1]
			});
			SOMBRA_DEBUG_LOG << "Updated NormalConstraint[" << i << "] between " << rb1 << " and " << rb2;
		}
	}


	void PhysicsManager::handleDisjointManifold(
		physics::RigidBody* rb1, physics::RigidBody* rb2,
		const collision::Manifold* manifold)
	{
		auto itPair = mManifoldConstraints.find(manifold);
		if (itPair != mManifoldConstraints.end()) {
			for (physics::NormalConstraint& constraint : itPair->second) {
				mPhysicsEngine.getConstraintManager().removeConstraint(&constraint);
			}

			SOMBRA_DEBUG_LOG << "Removed all the NormalConstraints (" << itPair->second.size() << ") between "
				<< rb1 << " and " << rb2;
			mManifoldConstraints.erase(itPair);
		}
		else {
			SOMBRA_ERROR_LOG << "Couldn't remove all the NormalConstraints between " << rb1 << " and " << rb2;
		}
	}

}
