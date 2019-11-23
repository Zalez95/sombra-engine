#include <glm/gtx/string_cast.hpp>
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

		// The RigidBody initial data is overridden by the entity one
		physics::RigidBody* rbPtr = rigidBody.get();
		rbPtr->getData().position		= entity->position;
		rbPtr->getData().linearVelocity	= entity->velocity;
		rbPtr->getData().orientation	= entity->orientation;
		rbPtr->synchWithData();

		// Add the RigidBody
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

		mPhysicsEngine.resetRigidBodiesState();

		SOMBRA_DEBUG_LOG << "Updating the RigidBodies";
		for (auto& pair : mEntityRBMap) {
			Entity* entity = pair.first;
			physics::RigidBody* rigidBody = pair.second.get();

			// Reset the Entity physics update
			entity->updated.reset( static_cast<int>(Entity::Update::Physics) );

			if (entity->updated.any()) {
				rigidBody->getData().position		= entity->position;
				rigidBody->getData().linearVelocity	= entity->velocity;
				rigidBody->getData().orientation	= entity->orientation;
				rigidBody->synchWithData();
			}
		}

		SOMBRA_DEBUG_LOG << "Integrating the RigidBodies";
		mPhysicsEngine.integrate(delta);

		SOMBRA_DEBUG_LOG << "Updating the Entities";
		for (auto& pair : mEntityRBMap) {
			Entity* entity					= pair.first;
			physics::RigidBody* rigidBody	= pair.second.get();

			if (rigidBody->checkState(physics::RigidBodyState::Integrated)) {
				entity->position	= rigidBody->getData().position;
				entity->velocity	= rigidBody->getData().linearVelocity;
				entity->orientation	= rigidBody->getData().orientation;
				entity->updated.set( static_cast<int>(Entity::Update::Physics) );
			}
		}

		SOMBRA_INFO_LOG << "End";
	}


	void PhysicsManager::doConstraints(float delta)
	{
		SOMBRA_INFO_LOG << "Start (" << delta << ")";

		mPhysicsEngine.resetRigidBodiesState();

		SOMBRA_DEBUG_LOG << "Updating the RigidBodies";
		for (auto& pair : mEntityRBMap) {
			Entity* entity = pair.first;
			physics::RigidBody* rigidBody = pair.second.get();

			// Skip the Entity physics change in the doDynamics step
			auto updatedWithoutPhysics = entity->updated;
			updatedWithoutPhysics.reset( static_cast<int>(Entity::Update::Physics) );

			if (updatedWithoutPhysics.any()) {
				rigidBody->getData().position		= entity->position;
				rigidBody->getData().linearVelocity	= entity->velocity;
				rigidBody->getData().orientation	= entity->orientation;
				rigidBody->synchWithData();
			}
		}

		SOMBRA_DEBUG_LOG << "Updating the NormalConstraints time";
		for (auto& pair : mManifoldConstraintsMap) {
			for (ContactConstraints& contactConstraints : pair.second) {
				contactConstraints.normalConstraint.setDeltaTime(delta);
			}
		}

		SOMBRA_DEBUG_LOG << "Solving the Constraints";
		mPhysicsEngine.solveConstraints(delta);

		SOMBRA_DEBUG_LOG << "Updating the Entities";
		for (auto& pair : mEntityRBMap) {
			Entity* entity = pair.first;
			physics::RigidBody* rigidBody = pair.second.get();

			if (rigidBody->checkState(physics::RigidBodyState::ConstraintsSolved)) {
				entity->position	= rigidBody->getData().position;
				entity->velocity	= rigidBody->getData().linearVelocity;
				entity->orientation	= rigidBody->getData().orientation;
				entity->updated.set( static_cast<int>(Entity::Update::Physics) );
			}
		}

		SOMBRA_DEBUG_LOG << "Putting the RigidBodies to sleep";
		mPhysicsEngine.checkSleepyRigidBodies(delta);

		SOMBRA_INFO_LOG << "End";
	}

// Private functions
	void PhysicsManager::onCollisionEvent(const CollisionEvent& event)
	{
		SOMBRA_TRACE_LOG << "Received CollisionEvent: " << event;

		Entity* entity1 = event.getEntity(0);
		Entity* entity2 = event.getEntity(1);
		const collision::Manifold* manifold = event.getManifold();

		if (entity1 && entity2 && manifold) {
			auto itEntityRB1 = mEntityRBMap.find(entity1);
			auto itEntityRB2 = mEntityRBMap.find(entity2);
			if ((itEntityRB1 != mEntityRBMap.end()) && (itEntityRB2 != mEntityRBMap.end())) {
				physics::RigidBody* rb1 = itEntityRB1->second.get();
				physics::RigidBody* rb2 = itEntityRB2->second.get();
				if ((rb1->getConfig().invertedMass != 0.0f) || (rb2->getConfig().invertedMass != 0.0f)) {
					SOMBRA_DEBUG_LOG << "Handling CollisionEvent between " << rb1 << " and " << rb2;

					if (manifold->state[collision::Manifold::State::Intersecting]) {
						handleIntersectingManifold(rb1, rb2, manifold);
					}
					else {
						handleDisjointManifold(manifold);
					}
				}
				else {
					SOMBRA_TRACE_LOG << "Skipping CollisionEvent between infinite mass RigidBodies " << rb1 << " and " << rb2;
				}
			}
			else {
				SOMBRA_WARN_LOG << "CollisionEvent Entities hasn't been added: " << event;
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
		ManifoldConstraints& manifoldConstraints = mManifoldConstraintsMap[manifold];

		bool updateFrictionMasses = true;
		if (manifold->contacts.size() > manifoldConstraints.size()) {
			float mu1 = rb1->getConfig().frictionCoefficient, mu2 = rb2->getConfig().frictionCoefficient;
			float mu = std::sqrt(mu1 * mu1 + mu2 * mu2);
			SOMBRA_DEBUG_LOG << "Using frictionCoefficient=" << mu;

			// Increase the number of constraints up to the number of contacts
			for (std::size_t i = manifoldConstraints.size(); i < manifold->contacts.size(); ++i) {
				auto itConstraint = manifoldConstraints.push_back({
					physics::NormalConstraint(
						{ rb1, rb2 }, kCollisionBeta, kCollisionRestitutionFactor,
						kCollisionSlopPenetration, kCollisionSlopRestitution
					),
					{
						physics::FrictionConstraint({ rb1, rb2 }, kFrictionGravityAcceleration, mu),
						physics::FrictionConstraint({ rb1, rb2 }, kFrictionGravityAcceleration, mu)
					}
				});

				mPhysicsEngine.getConstraintManager().addConstraint(&itConstraint->normalConstraint);
				mPhysicsEngine.getConstraintManager().addConstraint(&itConstraint->frictionConstraints[0]);
				mPhysicsEngine.getConstraintManager().addConstraint(&itConstraint->frictionConstraints[1]);

				SOMBRA_DEBUG_LOG << "Added ContactConstraint[" << i << "]";
			}
		}
		else if (manifold->contacts.size() < manifoldConstraints.size()) {
			// Decrease the number of constraints down to the number of contacts
			for (std::size_t i = manifoldConstraints.size(); i > manifold->contacts.size(); --i) {
				ContactConstraints& lastConstraints = manifoldConstraints.back();
				mPhysicsEngine.getConstraintManager().removeConstraint(&lastConstraints.normalConstraint);
				mPhysicsEngine.getConstraintManager().removeConstraint(&lastConstraints.frictionConstraints[0]);
				mPhysicsEngine.getConstraintManager().removeConstraint(&lastConstraints.frictionConstraints[1]);
				manifoldConstraints.pop_back();

				SOMBRA_DEBUG_LOG << "Removed ContactConstraint[" << i-1 << "]";
			}
		}
		else {
			updateFrictionMasses = false;
		}

		if (updateFrictionMasses) {
			// Update the friction constraint masses
			float averageMass = 2.0f / (rb1->getConfig().invertedMass + rb2->getConfig().invertedMass);
			float perContactMass = averageMass / manifoldConstraints.size();

			for (ContactConstraints& contactConstraints : manifoldConstraints) {
				contactConstraints.frictionConstraints[0].calculateConstraintBounds(perContactMass);
				contactConstraints.frictionConstraints[1].calculateConstraintBounds(perContactMass);
			}

			SOMBRA_DEBUG_LOG << "Updated FrictionConstraint masses to " << perContactMass;
		}

		// Update the constraints data
		for (std::size_t i = 0; i < manifold->contacts.size(); ++i) {
			const collision::Contact& contact = manifold->contacts[i];

			// Calculate the vectors that points from the RigidBodies center of
			// mass to their contact points
			glm::vec3 r1 = contact.worldPosition[0] - rb1->getData().position;
			glm::vec3 r2 = contact.worldPosition[1] - rb2->getData().position;

			// Calculate two tangent vectors to the Contact normal
			glm::vec3 vAxis(0.0f);
			auto absCompare = [](float f1, float f2) { return std::abs(f1) < std::abs(f2); };
			std::size_t iAxis = std::distance(&contact.normal.x, std::min_element(&contact.normal.x, &contact.normal.x + 3, absCompare));
			vAxis[iAxis] = 1.0f;

			glm::vec3 tangent1 = glm::cross(contact.normal, vAxis);
			glm::vec3 tangent2 = glm::cross(contact.normal, tangent1);

			manifoldConstraints[i].normalConstraint.setNormal(contact.normal);
			manifoldConstraints[i].normalConstraint.setConstraintVectors({ r1, r2 });
			manifoldConstraints[i].frictionConstraints[0].setTangent(tangent1);
			manifoldConstraints[i].frictionConstraints[0].setConstraintVectors({ r1, r2 });
			manifoldConstraints[i].frictionConstraints[1].setTangent(tangent2);
			manifoldConstraints[i].frictionConstraints[1].setConstraintVectors({ r1, r2 });

			SOMBRA_DEBUG_LOG << "Updated ContactConstraints[" << i << "]: "
				<< "r1=" << glm::to_string(r1) << ", " << "r2=" << glm::to_string(r2) << ", "
				<< "normal=" << glm::to_string(contact.normal) << ", "
				<< "tangent1=" << glm::to_string(tangent1) << " and tangent2=" << glm::to_string(tangent2);
		}
	}


	void PhysicsManager::handleDisjointManifold(const collision::Manifold* manifold)
	{
		auto itPair = mManifoldConstraintsMap.find(manifold);
		if (itPair != mManifoldConstraintsMap.end()) {
			for (ContactConstraints& constraints : itPair->second) {
				mPhysicsEngine.getConstraintManager().removeConstraint(&constraints.normalConstraint);
				mPhysicsEngine.getConstraintManager().removeConstraint(&constraints.frictionConstraints[0]);
				mPhysicsEngine.getConstraintManager().removeConstraint(&constraints.frictionConstraints[1]);
			}

			SOMBRA_DEBUG_LOG << "Removed all the ContactConstraints (" << itPair->second.size() << ")";
			mManifoldConstraintsMap.erase(itPair);
		}
		else {
			SOMBRA_WARN_LOG << "Doesn't exists any ContactConstraints";
		}
	}

}
