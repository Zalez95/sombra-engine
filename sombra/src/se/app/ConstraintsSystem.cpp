#include <glm/gtx/string_cast.hpp>
#include "se/utils/Log.h"
#include "se/physics/RigidBody.h"
#include "se/physics/PhysicsEngine.h"
#include "se/app/ConstraintsSystem.h"
#include "se/app/Application.h"
#include "se/app/TransformsComponent.h"
#include "se/app/events/CollisionEvent.h"

namespace se::app {

	ConstraintsSystem::ConstraintsSystem(Application& application) :
		ISystem(application.getEntityDatabase()), mApplication(application)
	{
		mManifoldConstraintIndicesMap.reserve(kMaxContacts / collision::Manifold::kMaxContacts);
		mContactNormalConstraints.reserve(kMaxContacts);
		mContactFrictionConstraints.reserve(2 * kMaxContacts);

		mApplication.getEventManager().subscribe(this, Topic::Collision);
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask().set<physics::RigidBody>());
	}


	ConstraintsSystem::~ConstraintsSystem()
	{
		auto& physicsEngine = *mApplication.getExternalTools().physicsEngine;
		for (auto& normalConstraint : mContactNormalConstraints) {
			physicsEngine.getConstraintManager().removeConstraint(&normalConstraint);
		}
		for (auto& frictionConstraint : mContactFrictionConstraints) {
			physicsEngine.getConstraintManager().removeConstraint(&frictionConstraint);
		}

		mEntityDatabase.removeSystem(this);
		mApplication.getEventManager().unsubscribe(this, Topic::Collision);
	}


	void ConstraintsSystem::notify(const IEvent& event)
	{
		tryCall(&ConstraintsSystem::onCollisionEvent, event);
	}


	void ConstraintsSystem::update()
	{
		SOMBRA_INFO_LOG << "Start";
		auto& physicsEngine = *mApplication.getExternalTools().physicsEngine;

		physicsEngine.resetRigidBodiesState();

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

		SOMBRA_DEBUG_LOG << "Updating the NormalConstraints time";
		for (auto& normalConstraint : mContactNormalConstraints) {
			normalConstraint.setDeltaTime(mDeltaTime);
		}

		SOMBRA_DEBUG_LOG << "Solving the Constraints";
		physicsEngine.solveConstraints(mDeltaTime);

		SOMBRA_DEBUG_LOG << "Updating the Entities";
		mEntityDatabase.iterateComponents<TransformsComponent, physics::RigidBody>(
			[this](Entity, TransformsComponent* transforms, physics::RigidBody* rigidBody) {
				if (rigidBody->checkState(physics::RigidBodyState::ConstraintsSolved)) {
					transforms->position	= rigidBody->getData().position;
					transforms->velocity	= rigidBody->getData().linearVelocity;
					transforms->orientation	= rigidBody->getData().orientation;

					transforms->updated.reset().set(static_cast<int>(TransformsComponent::Update::RigidBody));
				}
			},
			true
		);

		SOMBRA_DEBUG_LOG << "Putting the RigidBodies to sleep";
		physicsEngine.checkSleepyRigidBodies(mDeltaTime);

		SOMBRA_INFO_LOG << "End";
	}

// Private functions
	void ConstraintsSystem::onNewRigidBody(Entity entity, physics::RigidBody* rigidBody)
	{
		auto [transforms] = mEntityDatabase.getComponents<TransformsComponent>(entity, true);
		if (transforms) {
			transforms->updated.reset(static_cast<int>(TransformsComponent::Update::RigidBody));
		}

		SOMBRA_INFO_LOG << "Entity " << entity << " with RigidBody " << rigidBody << " added successfully";
	}


	void ConstraintsSystem::onRemoveRigidBody(Entity entity, physics::RigidBody* rigidBody)
	{
		// Remove the Constraints from the ConstraintManager
		auto& physicsEngine = *mApplication.getExternalTools().physicsEngine;
		physicsEngine.getConstraintManager().removeRigidBody(rigidBody);

		// Remove the Constraint indices
		for (auto it = mManifoldConstraintIndicesMap.begin(); it != mManifoldConstraintIndicesMap.end();) {
			if ((mContactNormalConstraints[it->second[0].iNormalConstraint].getRigidBody(0) == rigidBody)
				|| (mContactNormalConstraints[it->second[0].iNormalConstraint].getRigidBody(1) == rigidBody)
			) {
				for (const auto& constraintIndices : it->second) {
					mContactNormalConstraints.erase(mContactNormalConstraints.begin().setIndex(constraintIndices.iNormalConstraint));
					mContactFrictionConstraints.erase(mContactFrictionConstraints.begin().setIndex(constraintIndices.iFrictionConstraints[0]));
					mContactFrictionConstraints.erase(mContactFrictionConstraints.begin().setIndex(constraintIndices.iFrictionConstraints[1]));
				}

				it = mManifoldConstraintIndicesMap.erase(it);
			}
			else {
				++it;
			}
		}

		SOMBRA_INFO_LOG << "Entity " << entity << " with RigidBody " << rigidBody << " removed successfully";
	}


	void ConstraintsSystem::onCollisionEvent(const CollisionEvent& event)
	{
		SOMBRA_TRACE_LOG << "Received CollisionEvent: " << event;

		auto [rb1] = mEntityDatabase.getComponents<physics::RigidBody>(event.getEntity(0), true);
		auto [rb2] = mEntityDatabase.getComponents<physics::RigidBody>(event.getEntity(1), true);
		const collision::Manifold* manifold = event.getManifold();

		if (rb1 && rb2 && manifold) {
			if ((rb1->getConfig().invertedMass > 0.0f) || (rb2->getConfig().invertedMass > 0.0f)) {
				SOMBRA_DEBUG_LOG << "Handling CollisionEvent between "
					<< rb1 << " (p=" << glm::to_string(rb1->getData().position) << ", o=" << glm::to_string(rb1->getData().orientation) << ") and "
					<< rb2 << " (p=" << glm::to_string(rb2->getData().position) << ", o=" << glm::to_string(rb2->getData().orientation) << ")";

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
			SOMBRA_ERROR_LOG << "Wrong CollisionEvent data: " << event;
		}
	}


	void ConstraintsSystem::handleIntersectingManifold(
		physics::RigidBody* rb1, physics::RigidBody* rb2,
		const collision::Manifold* manifold
	) {
		auto& physicsEngine = *mApplication.getExternalTools().physicsEngine;
		auto& manifoldConstraintIndices = mManifoldConstraintIndicesMap[manifold];
		bool updateFrictionMasses = true;

		int contactsDiff = static_cast<int>(manifold->contacts.size()) - static_cast<int>(manifoldConstraintIndices.size());
		if (contactsDiff > 0) {
			if (mContactNormalConstraints.size() + contactsDiff <= kMaxContacts) {
				float mu1 = rb1->getConfig().frictionCoefficient, mu2 = rb2->getConfig().frictionCoefficient;
				float mu = std::sqrt(0.5f * (mu1 * mu1 + mu2 * mu2));
				SOMBRA_DEBUG_LOG << "Using frictionCoefficient=" << mu;

				// Increase the number of constraints up to the number of contacts
				for (std::size_t i = manifoldConstraintIndices.size(); i < manifold->contacts.size(); ++i) {
					auto itNormalConstraint = mContactNormalConstraints.emplace(
						std::array{ rb1, rb2 }, kCollisionBeta, kCollisionRestitutionFactor,
						kCollisionSlopPenetration, kCollisionSlopRestitution
					);
					auto itFrictionConstraint0 = mContactFrictionConstraints.emplace(std::array{ rb1, rb2 }, kFrictionGravityAcceleration, mu);
					auto itFrictionConstraint1 = mContactFrictionConstraints.emplace(std::array{ rb1, rb2 }, kFrictionGravityAcceleration, mu);

					physicsEngine.getConstraintManager().addConstraint(&(*itNormalConstraint));
					physicsEngine.getConstraintManager().addConstraint(&(*itFrictionConstraint0));
					physicsEngine.getConstraintManager().addConstraint(&(*itFrictionConstraint1));

					manifoldConstraintIndices.emplace_back(ContactConstraintIndices{
						itNormalConstraint.getIndex(), { itFrictionConstraint0.getIndex(), itFrictionConstraint1.getIndex() }
					});

					SOMBRA_DEBUG_LOG << "Added contact Constraints [" << i << "]";
				}
			}
			else {
				SOMBRA_WARN_LOG << "Maximum number of Contacts reached";
			}
		}
		else if (contactsDiff < 0) {
			// Decrease the number of constraints down to the number of contacts
			for (std::size_t i = manifoldConstraintIndices.size(); i > manifold->contacts.size(); --i) {
				ContactConstraintIndices& constraintIndices = manifoldConstraintIndices.back();
				physicsEngine.getConstraintManager().removeConstraint(&mContactNormalConstraints[constraintIndices.iNormalConstraint]);
				physicsEngine.getConstraintManager().removeConstraint(&mContactFrictionConstraints[constraintIndices.iFrictionConstraints[0]]);
				physicsEngine.getConstraintManager().removeConstraint(&mContactFrictionConstraints[constraintIndices.iFrictionConstraints[1]]);

				mContactNormalConstraints.erase(mContactNormalConstraints.begin().setIndex(constraintIndices.iNormalConstraint));
				mContactFrictionConstraints.erase(mContactFrictionConstraints.begin().setIndex(constraintIndices.iFrictionConstraints[0]));
				mContactFrictionConstraints.erase(mContactFrictionConstraints.begin().setIndex(constraintIndices.iFrictionConstraints[1]));

				manifoldConstraintIndices.pop_back();

				SOMBRA_DEBUG_LOG << "Removed contact Constraints [" << i-1 << "]";
			}
		}
		else {
			updateFrictionMasses = false;
		}

		if (updateFrictionMasses) {
			// Update the friction constraint masses
			float averageMass = 0.5f * (1.0f / rb1->getConfig().invertedMass + 1.0f / rb2->getConfig().invertedMass);
			float perContactMass = averageMass / manifoldConstraintIndices.size();

			for (ContactConstraintIndices& constraintIndices : manifoldConstraintIndices) {
				mContactFrictionConstraints[constraintIndices.iFrictionConstraints[0]].calculateConstraintBounds(perContactMass);
				mContactFrictionConstraints[constraintIndices.iFrictionConstraints[1]].calculateConstraintBounds(perContactMass);
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
			vAxis[static_cast<glm::vec3::length_type>(iAxis)] = 1.0f;

			glm::vec3 tangent1 = glm::normalize(glm::cross(contact.normal, vAxis));
			glm::vec3 tangent2 = glm::normalize(glm::cross(contact.normal, tangent1));

			mContactNormalConstraints[manifoldConstraintIndices[i].iNormalConstraint].setNormal(contact.normal);
			mContactNormalConstraints[manifoldConstraintIndices[i].iNormalConstraint].setConstraintVectors({ r1, r2 });
			mContactFrictionConstraints[manifoldConstraintIndices[i].iFrictionConstraints[0]].setTangent(tangent1);
			mContactFrictionConstraints[manifoldConstraintIndices[i].iFrictionConstraints[0]].setConstraintVectors({ r1, r2 });
			mContactFrictionConstraints[manifoldConstraintIndices[i].iFrictionConstraints[1]].setTangent(tangent2);
			mContactFrictionConstraints[manifoldConstraintIndices[i].iFrictionConstraints[1]].setConstraintVectors({ r1, r2 });

			SOMBRA_DEBUG_LOG << "Updated contact Constraints [" << i << "]: "
				<< "r1=" << glm::to_string(r1) << ", " << "r2=" << glm::to_string(r2) << ", "
				<< "normal=" << glm::to_string(contact.normal) << ", "
				<< "tangent1=" << glm::to_string(tangent1) << " and tangent2=" << glm::to_string(tangent2);
		}
	}


	void ConstraintsSystem::handleDisjointManifold(const collision::Manifold* manifold)
	{
		auto& physicsEngine = *mApplication.getExternalTools().physicsEngine;

		auto itPair = mManifoldConstraintIndicesMap.find(manifold);
		if (itPair != mManifoldConstraintIndicesMap.end()) {
			for (auto& constraintIndices : itPair->second) {
				physicsEngine.getConstraintManager().removeConstraint(&mContactNormalConstraints[constraintIndices.iNormalConstraint]);
				physicsEngine.getConstraintManager().removeConstraint(&mContactFrictionConstraints[constraintIndices.iFrictionConstraints[0]]);
				physicsEngine.getConstraintManager().removeConstraint(&mContactFrictionConstraints[constraintIndices.iFrictionConstraints[1]]);

				mContactNormalConstraints.erase(mContactNormalConstraints.begin().setIndex(constraintIndices.iNormalConstraint));
				mContactFrictionConstraints.erase(mContactFrictionConstraints.begin().setIndex(constraintIndices.iFrictionConstraints[0]));
				mContactFrictionConstraints.erase(mContactFrictionConstraints.begin().setIndex(constraintIndices.iFrictionConstraints[1]));
			}

			SOMBRA_DEBUG_LOG << "Removed all the contact Constraints (" << itPair->second.size() << ")";
			mManifoldConstraintIndicesMap.erase(itPair);
		}
		else {
			SOMBRA_WARN_LOG << "Doesn't exists any contact Constraints";
		}
	}

}
