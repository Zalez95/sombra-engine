#include "fe/physics/PhysicsEngine.h"
#include <limits>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "fe/physics/PhysicsEntity.h"

namespace fe { namespace physics {

// Constants definition
	const ConstraintBounds PhysicsEngine::CONSTRAINT_BOUNDS(0.0f, std::numeric_limits<float>::max());
	const float PhysicsEngine::CONSTRAINT_BETA = 1.1f;

// Public functions
	void PhysicsEngine::update(float delta)
	{
		mForceManager.applyForces();
		integrate(delta);
		collide(delta);
	}


	void PhysicsEngine::addPhysicsEntity(PhysicsEntity* entity)
	{
		if (!entity) { return; }

		mPhysicsEntities.emplace(entity);

		if (collision::Collider* collider = entity->getCollider()) {
			mCollisionDetector.addCollider(collider);
			mColliderEntityMap.emplace(collider, entity);
		}
	}


	void PhysicsEngine::removePhysicsEntity(PhysicsEntity* entity)
	{
		if (!entity) { return; }

		mPhysicsEntities.erase(entity);

		if (collision::Collider* collider = entity->getCollider()) {
			mCollisionDetector.removeCollider(collider);
			for (auto it = mColliderEntityMap.begin(); it != mColliderEntityMap.end(); ) {
				if (it->second == entity) {
					mColliderEntityMap.erase(it);
					break;
				}
				else {
					++it;
				}
			}
		}
	}

// Private functions
	void PhysicsEngine::integrate(float delta)
	{
		for (PhysicsEntity* physicsEntity : mPhysicsEntities) {
			// Update the RigidBody data
			RigidBody* rigidBody = physicsEntity->getRigidBody();
			rigidBody->integrate(delta);

			// Update the Collider data
			if (collision::Collider* collider = physicsEntity->getCollider()) {
				glm::mat4 colliderOffset = physicsEntity->getColliderOffset();
				collider->setTransforms(rigidBody->getTransformsMatrix() * colliderOffset);
			}
		}
	}


	void PhysicsEngine::collide(float delta)
	{
		mCollisionDetector.update();

		// Update the contact constraints (or create new ones) with the data
		// of the collision contacts
		std::set<collision::Contact*> active;
		for (collision::Manifold* manifold : mCollisionDetector.getCollisionManifolds()) {
			for (collision::Contact& contact : manifold->getContacts()) {
				// Find the Constraint of the Contact or create a new one if
				// it doesn't exist yet
				auto it = mContactConstraints.find(&contact);
				if (it == mContactConstraints.end()) {
					RigidBody* rb1 = mColliderEntityMap[manifold->getFirstCollider()]->getRigidBody();
					RigidBody* rb2 = mColliderEntityMap[manifold->getSecondCollider()]->getRigidBody();

					it = mContactConstraints.emplace(
						std::piecewise_construct,
						std::forward_as_tuple(&contact),
						std::forward_as_tuple(&CONSTRAINT_BOUNDS, std::array<RigidBody*, 2>{ rb1, rb2 }, CONSTRAINT_BETA)
					).first;
					mConstraintManager.addConstraint(&it->second);
				}

				// Update the constraint data
				it->second.setConstraintPoints({contact.getLocalPosition(0), contact.getLocalPosition(1)});
				it->second.setConstraintNormal(contact.getNormal());
				it->second.setDeltaTime(delta);
				it->second.increaseK();

				// Mark the constraint as active
				active.insert(&contact);
			}
		}

		// Remove the inactive contact constraints
		for (auto it = mContactConstraints.begin(); it != mContactConstraints.end();) {
			if (active.find(it->first) == active.end()) {
				mConstraintManager.removeConstraint(&it->second);
				it = mContactConstraints.erase(it);
			}
			else {
				++it;
			}
		}

		mConstraintManager.update(delta);
	}

}}
