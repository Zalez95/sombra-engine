#ifndef RIGID_BODY_H
#define RIGID_BODY_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace physics {

	/**
	 * Class RigidBody, a Rigid Body is the basic object of the Physics System
	 * <br>A Rigid Body holds position and orientation.
	 */
	class RigidBody
	{
	private:	// Attributes
		/** All the linear data of the RigidBody used for integrate the
		 * position */
		struct
		{
			/** The inverse of the mass. We store the mass inverted because
			 * it's more useful for storing object with infinite mass */
			float mInvertedMass;

			/** The linear position of the origin (center of mass) of the
			 * RigidBody in world space */
			glm::vec3 mPosition;

			/** The linear velocity of the RigidBody in world space */
			glm::vec3 mVelocity;

			/** The factor by we are going to slow down the linear velocity of
			 * the RigidBody in each integrate call so it doesn't move
			 * forever */
			float mSlowDown;
		
			/** A vector with the sum of all the forces currently applied to
			 * the RigidBody */
			glm::vec3 mForceSum;
		} mLinearData;

		/** All the angular data if the RigidBody used for integrate the 
		 * orientation */
		struct
		{
			/** The inertia tensor of the RigidBody. It is a 3x3 matrix in
			 * Body Space that stores all the moments of inertia of a
			 * RigidBody.
			 * <br>As with the mass, the inertia tensor is stored inverted
			 * so we don't need to inverse the matrix in later calculus. */
			glm::mat3 mInvertedInertiaTensor;

			/** The inertia tensor in world space
			 * @see		mInvertedInertiaTensor
			 * @note	is a mat3 because it doesn't matter the location, only
			 *			the orientation */
			glm::mat3 mInvertedInertiaTensorWorld;

			/** The orientation of the RigidBody in world space */
			glm::quat mOrientation;

			/** The angular velocity of the RigidBody in world space */
			glm::vec3 mVelocity;

			/** The factor by we are going to slow down the angular velocity of
			 * the RigidBody in each integrate call so it doesn't rotate
			 * forever */
			float mSlowDown;

			/** A vector with the sum of all the torques currently applied to
			 * the RigidBody */
			glm::vec3 mTorqueSum;
		} mAngularData;

		/** The matrix that holds all the current tranformations of the
		 * RigidBody (translation and orientation) in world space to avoid
		 * recalculating the position and orientation */
		glm::mat4 mTransformsMatrix;

	public:		// Functions
		/** Creates a new RigidBody with infinite mass
		 * 
		 * @param	position position the initial position of the RigidBody
		 *			in world space
		 * @param	orientation the initial orientation of the RigidBody
		 *			in world space */
		RigidBody(const glm::vec3& position, const glm::quat& orientation);

		/** Creates a new RigidBody
		 *
		 * @param	mass the mass of the RigidBody
		 * @param	linearSlowDown the factor by we are going to slow down the
		 *			linear velocity
		 * @param	inertiaTensor a 3x3 matrix that stores all the moments of
		 *			inertia of the RigidBody
		 * @param	angularSlowDown the factor by we are going to slow down the
		 *			angular velocity
		 * @param	position the initial position of the RigidBody in world
		 *			space
		 * @param	orientation the initial orientation of the RigidBody in
		 *			world space */
		RigidBody(
			float mass, float linearSlowDown,
			const glm::mat3& inertiaTensor, float angularSlowDown,
			const glm::vec3& position, const glm::quat& orientation
		);

		/** Class destructor */
		~RigidBody() {};

		/** @return	the inverted mass (1/mass) of the RigidBody */
		inline float getInvertedMass() const
		{ return mLinearData.mInvertedMass; };

		/** @return	true if the RigidBody has Finite Mass, false otherwise  */
		inline bool hasFiniteMass() const
		{ return mLinearData.mInvertedMass > 0; };

		/** @return	the current linear position of the RigidBody */
		inline glm::vec3 getPosition() const { return mLinearData.mPosition; };

		/** Sets the position of the RigidBody
		 *
		 * @param	position the new position of the RigidBody */
		inline void setPosition(const glm::vec3& position)
		{ mLinearData.mPosition = position; };

		/** @return	the current linear velocity of the RigidBody */
		inline glm::vec3 getVelocity() const { return mLinearData.mVelocity; };

		/** Sets the linear velocity of the RigidBody
		 *
		 * @param	velocity the new linear velocity of the RigidBody */
		inline void setVelocity(const glm::vec3& velocity)
		{ mLinearData.mVelocity = velocity; };

		/** @return	the inverted inertiaTensor matrix of the RigidBody */
		inline glm::mat3 getInvertedInertiaTensor() const
		{ return mAngularData.mInvertedInertiaTensor; };

		/** @return	the current orientation of the RigidBody */
		inline glm::quat getOrientation() const
		{ return mAngularData.mOrientation; };

		/** Sets the orientation of the RigidBody
		 *
		 * @param	orientation the new orientation of the RigidBody */
		inline void setOrientation(const glm::quat& orientation)
		{ mAngularData.mOrientation = orientation; };

		/** @return	the current angular velocity of the RigidBody */
		inline glm::vec3 getAngularVelocity() const
		{ return mAngularData.mVelocity; };

		/** Sets the angular velocity of the RigidBody
		 *
		 * @param	angularVelocity the new angular velocity of the RigidBody */
		inline void setAngularVelocity(const glm::vec3& angularVelocity)
		{ mAngularData.mVelocity = angularVelocity; };

		/** @return	the transformations matrix of the Rigid Body */
		inline glm::mat4 getTransformsMatrix() const
		{ return mTransformsMatrix; };

		/** Applies the given force to the center of mass of the RigidBody
		 * 
		 * @param	force the force that we want to apply to the RigidBody in
		 *			world space */
		void addForce(const glm::vec3& force);

		/** Applies the given force to the given point of the RigidBody
		 * 
		 * @param	force the Force that we want to apply in World Space
		 * @param	point the point of the RigidBody where we are going to
		 *			apply the force in World Space */
		void addForceAtPoint(const glm::vec3& force, const glm::vec3& point);
		
		/** Applies the given force to the given point of the RigidBody
		 * 
		 * @param	force the Force that we want to apply in World Space
		 * @param	point the point of the RigidBody where we are going to
		 *			apply the force in Local Space */
		void addForceAtLocalPoint(
			const glm::vec3& force,
			const glm::vec3& point
		);

		/** Cleans all the forces applied to the current RigidBody si they will
		 * no longer change its movement */
		void cleanForces();
		
		/** Integrates the all the linear and angular data of the RigidBody
		 * by the given amount of time
		 *
		 * @param	delta the time by we will integrate the attributes of the
		 *			RigidBody */
		void integrate(float delta);
	private:
		/** Updates the transformations matrix with the current data of the
		 * RigidBody */
		void updateTransformsMatrix();

		/** Updates the Inertia Tensor in world coordinates with the current data
		 * of the RigidBody */
		void updateInertiaTensorWorld();
	};

}

#endif		// RIGID_BODY_H
