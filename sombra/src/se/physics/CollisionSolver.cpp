#include <glm/gtx/string_cast.hpp>
#include "se/utils/Log.h"
#include "se/physics/RigidBodyWorld.h"
#include "se/physics/CollisionSolver.h"

namespace se::physics {

	CollisionSolver::CollisionSolver(RigidBodyWorld& parentWorld) : mParentWorld(parentWorld)
	{
		mManifoldConstraintIndicesMap.reserve(4 * mParentWorld.getProperties().maxCollidingRBs / Manifold::kMaxContacts);
		mContactNormalConstraints.reserve(4 * mParentWorld.getProperties().maxCollidingRBs);
		mContactFrictionConstraints.reserve(8 * mParentWorld.getProperties().maxCollidingRBs);
	}


	CollisionSolver::~CollisionSolver()
	{
		for (auto& normalConstraint : mContactNormalConstraints) {
			mParentWorld.getConstraintManager().removeConstraint(&normalConstraint);
		}
		for (auto& frictionConstraint : mContactFrictionConstraints) {
			mParentWorld.getConstraintManager().removeConstraint(&frictionConstraint);
		}
	}


	void CollisionSolver::onCollision(const Manifold& manifold)
	{
		RigidBody* rb1 = manifold.colliders[0]->getParent();
		RigidBody* rb2 = manifold.colliders[1]->getParent();

		if (rb1 && rb2) {
			if ((rb1->getProperties().type == RigidBodyProperties::Type::Dynamic)
				|| (rb2->getProperties().type == RigidBodyProperties::Type::Dynamic)
			) {
				SOMBRA_DEBUG_LOG << "Handling CollisionEvent between "
					<< rb1 << " (p=" << glm::to_string(rb1->getState().position) << ", o=" << glm::to_string(rb1->getState().orientation) << ") and "
					<< rb2 << " (p=" << glm::to_string(rb2->getState().position) << ", o=" << glm::to_string(rb2->getState().orientation) << ")";

				if (manifold.state[Manifold::State::Intersecting]) {
					handleIntersectingManifold(manifold);
				}
				else {
					handleDisjointManifold(manifold);
				}
			}
			else {
				SOMBRA_TRACE_LOG << "Skipping CollisionEvent between static RigidBodies " << rb1 << " and " << rb2;
			}
		}
	}


	void CollisionSolver::removeRigidBody(const RigidBody* rigidBody)
	{
		std::scoped_lock lock(mMutex);
		for (auto itPair = mManifoldConstraintIndicesMap.begin(); itPair != mManifoldConstraintIndicesMap.end();) {
			if ((rigidBody == itPair->first->colliders[0]->getParent())
				|| (rigidBody == itPair->first->colliders[1]->getParent())
			) {
				for (auto& constraintIndices : itPair->second) {
					mParentWorld.getConstraintManager().removeConstraint(&mContactNormalConstraints[constraintIndices.iNormalConstraint]);
					mParentWorld.getConstraintManager().removeConstraint(&mContactFrictionConstraints[constraintIndices.iFrictionConstraints[0]]);
					mParentWorld.getConstraintManager().removeConstraint(&mContactFrictionConstraints[constraintIndices.iFrictionConstraints[1]]);

					mContactNormalConstraints.erase(mContactNormalConstraints.begin().setIndex(constraintIndices.iNormalConstraint));
					mContactFrictionConstraints.erase(mContactFrictionConstraints.begin().setIndex(constraintIndices.iFrictionConstraints[0]));
					mContactFrictionConstraints.erase(mContactFrictionConstraints.begin().setIndex(constraintIndices.iFrictionConstraints[1]));
				}

				itPair = mManifoldConstraintIndicesMap.erase(itPair);
			}
			else {
				++itPair;
			}
		}
	}


	void CollisionSolver::update(float deltaTime)
	{
		std::scoped_lock lock(mMutex);
		for (auto& normalConstraint : mContactNormalConstraints) {
			normalConstraint.setDeltaTime(deltaTime);
		}
	}

// Private functions
	void CollisionSolver::handleIntersectingManifold(const Manifold& manifold)
	{
		std::scoped_lock lock(mMutex);

		RigidBody* rb1 = manifold.colliders[0]->getParent();
		RigidBody* rb2 = manifold.colliders[1]->getParent();
		auto& manifoldConstraintIndices = mManifoldConstraintIndicesMap[&manifold];
		bool updateFrictionMasses = false;

		int contactsDiff = static_cast<int>(manifold.contacts.size()) - static_cast<int>(manifoldConstraintIndices.size());
		if (contactsDiff > 0) {
			if (mContactNormalConstraints.size() + contactsDiff <= 4 * mParentWorld.getProperties().maxCollidingRBs) {
				float mu1 = rb1->getProperties().frictionCoefficient, mu2 = rb2->getProperties().frictionCoefficient;
				float mu = std::sqrt(0.5f * (mu1 * mu1 + mu2 * mu2));
				SOMBRA_DEBUG_LOG << "Using frictionCoefficient=" << mu;

				// Increase the number of constraints up to the number of contacts
				for (std::size_t i = manifoldConstraintIndices.size(); i < manifold.contacts.size(); ++i) {
					auto itNormalConstraint = mContactNormalConstraints.emplace(
						std::array{ rb1, rb2 },
						mParentWorld.getProperties().collisionBeta,
						mParentWorld.getProperties().collisionRestitutionFactor,
						mParentWorld.getProperties().collisionSlopPenetration,
						mParentWorld.getProperties().collisionSlopRestitution
					);
					auto itFrictionConstraint0 = mContactFrictionConstraints.emplace(
						std::array{ rb1, rb2 }, mParentWorld.getProperties().frictionGravityAcceleration, mu
					);
					auto itFrictionConstraint1 = mContactFrictionConstraints.emplace(
						std::array{ rb1, rb2 }, mParentWorld.getProperties().frictionGravityAcceleration, mu
					);

					mParentWorld.getConstraintManager().addConstraint(&(*itNormalConstraint));
					mParentWorld.getConstraintManager().addConstraint(&(*itFrictionConstraint0));
					mParentWorld.getConstraintManager().addConstraint(&(*itFrictionConstraint1));

					manifoldConstraintIndices.emplace_back(ContactConstraintIndices{
						itNormalConstraint.getIndex(), { itFrictionConstraint0.getIndex(), itFrictionConstraint1.getIndex() }
					});

					SOMBRA_DEBUG_LOG << "Added contact Constraints [" << i << "]";
				}

				updateFrictionMasses = true;
			}
			else {
				SOMBRA_WARN_LOG << "Maximum number of Contacts reached";
			}
		}
		else if (contactsDiff < 0) {
			// Decrease the number of constraints down to the number of contacts
			for (std::size_t i = manifoldConstraintIndices.size(); i > manifold.contacts.size(); --i) {
				ContactConstraintIndices& constraintIndices = manifoldConstraintIndices.back();
				mParentWorld.getConstraintManager().removeConstraint(&mContactNormalConstraints[constraintIndices.iNormalConstraint]);
				mParentWorld.getConstraintManager().removeConstraint(&mContactFrictionConstraints[constraintIndices.iFrictionConstraints[0]]);
				mParentWorld.getConstraintManager().removeConstraint(&mContactFrictionConstraints[constraintIndices.iFrictionConstraints[1]]);

				mContactNormalConstraints.erase(mContactNormalConstraints.begin().setIndex(constraintIndices.iNormalConstraint));
				mContactFrictionConstraints.erase(mContactFrictionConstraints.begin().setIndex(constraintIndices.iFrictionConstraints[0]));
				mContactFrictionConstraints.erase(mContactFrictionConstraints.begin().setIndex(constraintIndices.iFrictionConstraints[1]));

				manifoldConstraintIndices.pop_back();

				SOMBRA_DEBUG_LOG << "Removed contact Constraints [" << i-1 << "]";
			}

			updateFrictionMasses = true;
		}

		if (updateFrictionMasses) {
			// Update the friction constraint masses
			float averageMass = 0.5f * (1.0f / rb1->getProperties().invertedMass + 1.0f / rb2->getProperties().invertedMass);
			float perContactMass = averageMass / manifoldConstraintIndices.size();

			for (ContactConstraintIndices& constraintIndices : manifoldConstraintIndices) {
				mContactFrictionConstraints[constraintIndices.iFrictionConstraints[0]].calculateConstraintBounds(perContactMass);
				mContactFrictionConstraints[constraintIndices.iFrictionConstraints[1]].calculateConstraintBounds(perContactMass);
			}

			SOMBRA_DEBUG_LOG << "Updated FrictionConstraint masses to " << perContactMass;
		}

		// Update the constraints data
		for (std::size_t i = 0; i < manifold.contacts.size(); ++i) {
			const Contact& contact = manifold.contacts[i];

			// Calculate the vectors that points from the RigidBodies center of
			// mass to their contact points
			glm::vec3 r1 = contact.worldPosition[0] - rb1->getState().position;
			glm::vec3 r2 = contact.worldPosition[1] - rb2->getState().position;

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


	void CollisionSolver::handleDisjointManifold(const Manifold& manifold)
	{
		std::scoped_lock lock(mMutex);

		auto itPair = mManifoldConstraintIndicesMap.find(&manifold);
		if (itPair != mManifoldConstraintIndicesMap.end()) {
			for (auto& constraintIndices : itPair->second) {
				mParentWorld.getConstraintManager().removeConstraint(&mContactNormalConstraints[constraintIndices.iNormalConstraint]);
				mParentWorld.getConstraintManager().removeConstraint(&mContactFrictionConstraints[constraintIndices.iFrictionConstraints[0]]);
				mParentWorld.getConstraintManager().removeConstraint(&mContactFrictionConstraints[constraintIndices.iFrictionConstraints[1]]);

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
