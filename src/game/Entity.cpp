#include "Entity.h"

namespace game {

// Static variables definition
	const glm::vec3 Entity::mDefaultForwardVector	= -glm::vec3(0, 0, 1);
	const glm::vec3 Entity::mDefaultUpVector		= glm::vec3(0, 1, 0);

// Public functions
	void Entity::synch()
	{
		if (mPhysicsEntity) {
			physics::RigidBody* rigidBody = mPhysicsEntity->getRigidBody();
			glm::vec3 position		= rigidBody->getPosition();
			glm::quat orientation	= rigidBody->getOrientation();

			if (mGraphicsData.mCamera) {
				glm::vec3 forwardVector = mDefaultForwardVector * orientation;
				glm::vec3 upVector		= mDefaultUpVector;
				
				mGraphicsData.mCamera->setPosition(position);
				mGraphicsData.mCamera->setTarget(position + forwardVector);
				mGraphicsData.mCamera->setUp(upVector);
			}
			if (mGraphicsData.mPointLight) {
				mGraphicsData.mPointLight->setPosition(position);
			}
			if (mGraphicsData.mRenderable3D) {
				mGraphicsData.mRenderable3D->setModelMatrix(rigidBody->getTransformsMatrix());
			}
		}
	}

}
