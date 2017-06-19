#include "Particle.h"
#include <glm/gtc/matrix_transform.hpp>

namespace physics {
	
	Particle::Particle(float mass, float slowDown)
	{
		mInvertedMass	= 1.0f / mass;
		mSlowDown		= slowDown;
	}


	void Particle::addForce(const glm::vec3& force)
	{ 
		mForceSum += force;
	}


	void Particle::cleanForces()
	{
		mForceSum = glm::vec3();
	}


	void Particle::integrate(float delta)
	{
		// Update the Position
		glm::vec3 curAcceleration = mInvertedMass * mForceSum;
		mVelocity = mVelocity * glm::pow(mSlowDown, delta) + curAcceleration * delta;
		mPosition += mVelocity * delta;

		// Update the derived data
		updateTransformsMatrix();
	}


	void Particle::updateTransformsMatrix()
	{
		mTransformsMatrix = glm::translate(glm::mat4(), mPosition);
	}

}
