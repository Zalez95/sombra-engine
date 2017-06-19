#ifndef PARTICLE_H
#define PARTICLE_H

#include <glm/glm.hpp>

namespace physics {

	/**
	 * Class Particle, a particle is the simplest object that we can simulate
	 * in the physics system.
	 * <br>A Particle holds position but no orientation.
	 */
	class Particle
	{
	private:	// Attributes
		/** The inverse of the mass. We store the mass inverted because it's
		 * more useful for storing object with infinite mass (immovable) */
		float mInvertedMass;

		/** The position of the particle in world space */
		glm::vec3 mPosition;

		/** The linear velocity of the particle in world space */
		glm::vec3 mVelocity;

		/** The sum of all the forces applied to the Particle */
		glm::vec3 mForceSum;

		/** The factor by we are going to slow down the velocity of the
		 * particle in each integrate call so it doesn't move forever */
		float mSlowDown;

		/** The matrix that holds all the current tranformations of the
		 * Particle (only translation) in world space to avoid recalculating
		 * it */
		glm::mat4 mTransformsMatrix;

	public:		// Functions
		/** Creates a new Particle
		 * 
		 * @param	mass the mass of the particle
		 * @param	position the initial position of the particle
		 * @param	slowDown the factor by we are going to slow down the
		 * 			velocity in each update */
		Particle(float mass, float slowDown);

		/** Class destructor */
		~Particle() {};

		/** @return	the mass of the Particle */
		inline float getMass() const { return 1.0f / mInvertedMass; };

		/** @return	true if the Particle has Finite Mass, false otherwise */
		inline bool hasFiniteMass() const
		{ return mInvertedMass > 0; };
		
		/** @return	the transformations matrix of the Particle */
		inline glm::mat4 getTransformsMatrix() const
		{ return mTransformsMatrix; };

		/** @return the current position of the Particle */
		inline glm::vec3 getPosition() const { return mPosition; };
 
		/** Adds the given force to the particle
		 * 
		 * @param force the force that we want to add to the particle */
		void addForce(const glm::vec3& force);

		/** Cleans all the forces applied to the current Particle si they will
		 * no longer change its movement */
		void cleanForces();

		/** Integrates the acceleration, velocity and position of the particle
		 * by the given amount of time using the Newton-Euler method
		 * 
		 * @param	delta the time by we will integrate the attributes of the
		 *			Particle */
		void integrate(float delta);

		/** Updates the transformations matrix with the current data of the
		 * Particle */
		void updateTransformsMatrix();
	};

}

#endif		// PARTICLE_H
