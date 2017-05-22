#include "PhysicsEngine.h"
#include <glm/gtc/matrix_transform.hpp>
#include "../utils/Logger.h"
#include "PhysicsEntity.h"

namespace physics {

	void PhysicsEngine::update(float delta)
	{
		mForceManager.applyForces();
		integrate(delta);
		// TODO: coarseCollisionDetection();
		// TODO: fineCollisionDetection();
		collide();
	}


	void PhysicsEngine::addPhysicsEntity(PhysicsEntity* entity)
	{
		if (entity != nullptr) {
			mPhysicsEntities.push_back(entity);
		}
	}


	void PhysicsEngine::removePhysicsEntity(PhysicsEntity* entity)
	{
		for (auto it = mPhysicsEntities.begin(); it != mPhysicsEntities.end(); ++it) {
			if ((*it) == entity) {
				mPhysicsEntities.erase(it);
				break;
			}
		}
	}

// Private functions
	void PhysicsEngine::integrate(float delta)
	{
		for (auto it = mPhysicsEntities.begin(); it != mPhysicsEntities.end(); ++it) {
            switch ((*it)->getType()) {
				case PhysicsEntityType::PARTICLE:
				{
					RigidBody* particle		= (*it)->getRigidBody();
					Collider* collider			= (*it)->getCollider();
					glm::mat4 colliderOffset	= (*it)->getColliderOffset();

					particle->integrate(delta);
					collider->setTransforms(colliderOffset * glm::translate(glm::mat4(), particle->getPosition()));
					break;
				}
	            case PhysicsEntityType::RIGID_BODY:
				{
					RigidBody* rigidBody		= (*it)->getRigidBody();
					Collider* collider			= (*it)->getCollider();
					glm::mat4 colliderOffset	= (*it)->getColliderOffset();

					rigidBody->integrate(delta);
					collider->setTransforms(colliderOffset * glm::translate(glm::mat4(), rigidBody->getPosition()));
					break;
				}
			}
		}
	}


	void PhysicsEngine::collide()
	{
		for (auto it1 = mPhysicsEntities.begin(); it1 != mPhysicsEntities.end(); it1++) {
			for (auto it2 = it1 + 1; it2 != mPhysicsEntities.end(); it2++) {
				const Collider* collider1 = (*it1)->getCollider();
				const Collider* collider2 = (*it2)->getCollider();

				if (mCollisionDetector.collide(collider1, collider2).size() > 0) {
					// TODO: Collision resolution
					Logger::writeLog(LogType::DEBUG, "collision");

					if ((*it1)->getType() == PhysicsEntityType::RIGID_BODY
						&& (*it2)->getType() == PhysicsEntityType::RIGID_BODY
					) {
						RigidBody* rb1 = (*it1)->getRigidBody();
						RigidBody* rb2 = (*it2)->getRigidBody();
						rb1->setVelocity(-rb1->getVelocity());
						rb2->setVelocity(-rb2->getVelocity());
					}
				}
			}
		}
	}

}
