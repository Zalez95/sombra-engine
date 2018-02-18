#ifndef RIGID_BODY_H
#define RIGID_BODY_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace fe { namespace physics {

	/**
	 * Class RigidBody, a Rigid Body is the basic object of the Physics System
	 * <br>A Rigid Body holds position and orientation.
	 */
	class RigidBody
	{
	private:	// Attributes
		/** The linear position of the origin (center of mass) of the
		 * RigidBody in world space */
		glm::vec3 mPosition;

		/** The linear velocity of the RigidBody in world space */
		glm::vec3 mLinearVelocity;

		/** The linear acceleration of the RigidBody in world space */
		glm::vec3 mLinearAcceleration;

		/** A vector with the sum of all the forces currently applied to
		 * the RigidBody */
		glm::vec3 mForceSum;

		/** The inverse of the mass. We store the mass inverted because
		 * it's more useful for storing object with infinite mass */
		float mInvertedMass;

		/** The factor by the linear velocity of the RigidBody is going to
		 * be slowed down each time the integrate method is called so the
		 * RigidBody doesn't move forever */
		float mLinearSlowDown;

		/** The orientation of the RigidBody in world space */
		glm::quat mOrientation;

		/** The angular velocity of the RigidBody in world space */
		glm::vec3 mAngularVelocity;

		/** The angular acceleration of the RigidBody in world space */
		glm::vec3 mAngularAcceleration;

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

		/** The factor by the angular velocity of the RigidBody is going to
		 * be slowed down each time the integrate method is called so the
		 * RigidBody doesn't move forever */
		float mAngularSlowDown;

		/** A vector with the sum of all the torques currently applied to
		 * the RigidBody */
		glm::vec3 mTorqueSum;

		/** The matrix that holds all the current tranformations of the
		 * RigidBody (translation and orientation) in world space to avoid
		 * recalculating the position and orientation */
		glm::mat4 mTransformsMatrix;

	public:		// Functions
		/** Creates a new RigidBody with infinite mass located at the origin
		 * of coordinates */
		RigidBody();

		/** Creates a new RigidBody located at the origin of coordinates
		 *
		 * @param	mass the mass of the RigidBody
		 * @param	linearSlowDown the factor by we are going to slow down the
		 *			linear velocity
		 * @param	inertiaTensor a 3x3 matrix that stores all the moments of
		 *			inertia of the RigidBody
		 * @param	angularSlowDown the factor by we are going to slow down the
		 *			angular velocity */
		RigidBody(
			float mass, float linearSlowDown,
			const glm::mat3& inertiaTensor, float angularSlowDown
		);

		/** Class destructor */
		~RigidBody() {};

		/** @return	the position of the RigidBody */
		inline glm::vec3 getPosition() const { return mPosition; };

		/** Sets the position of the RigidBody
		 * @param	position the new position of the RigidBody
		 * @note	after changing the position you must call the updateData
		 *			method */
		inline void setPosition(const glm::vec3& position)
		{ mPosition = position; };

		/** @return	the linear velocity of the RigidBody */
		inline glm::vec3 getLinearVelocity() const { return mLinearVelocity; };

		/** Sets the linear velocity of the RigidBody
		 * @param	velocity the new linear velocity of the RigidBody */
		inline void setLinearVelocity(const glm::vec3& velocity)
		{ mLinearVelocity = velocity; };

		/** @return	the velocity of the RigidBody */
		inline glm::vec3 getLinearAcceleration() const
		{ return mLinearAcceleration; };

		/** @return	the sum of forces currently applied to the RigidBody */
		inline glm::vec3 getForceSum() const { return mForceSum; };

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

		/** Cleans all the forces applied to the current RigidBody so they will
		 * no longer change its movement */
		void cleanForces();

		/** @return	the mass of the RigidBody */
		inline float getMass() const { return 1.0f / mInvertedMass; };

		/** @return	the inverted mass (1/mass) of the RigidBody */
		inline float getInvertedMass() const { return mInvertedMass; };

		/** @return	true if the RigidBody has Finite Mass, false otherwise */
		inline bool hasFiniteMass() const { return mInvertedMass > 0; };

		/** @return	the orientation of the RigidBody */
		inline glm::quat getOrientation() const { return mOrientation; };

		/** Sets the orientation of the RigidBody
		 * @param	orientation the new orientation of the RigidBody
		 * @note	after changing the orientation you must call the updateData
		 *			method */
		inline void setOrientation(const glm::quat& orientation)
		{ mOrientation = orientation; };

		/** @return	the angular velocity of the RigidBody */
		inline glm::vec3 getAngularVelocity() const
		{ return mAngularVelocity; };

		/** Sets the angular velocity of the RigidBody
		 * @param	velocity the new angular velocity of the RigidBody */
		inline void setAngularVelocity(const glm::vec3& velocity)
		{ mAngularVelocity = velocity; };

		/** @return	the velocity of the RigidBody */
		inline glm::vec3 getAngularAcceleration() const
		{ return mAngularAcceleration; };

		/** @return	the sum of torques of the forces currently applied to the
		 *			RigidBody */
		inline glm::vec3 getTorqueSum() const { return mTorqueSum; };

		/** @return	the inverted inertiaTensor matrix of the RigidBody */
		inline glm::mat3 getInvertedInertiaTensor() const
		{ return mInvertedInertiaTensor; };

		/** @return	the transformations matrix of the Rigid Body */
		inline glm::mat4 getTransformsMatrix() const
		{ return mTransformsMatrix; };

		/** Integrates the all the linear and angular data of the RigidBody
		 * by the given amount of time
		 *
		 * @param	delta the time by we will integrate the attributes of the
		 *			RigidBody in seconds */
		void integrate(float delta);

		/** Updates the RigidBody's internal data with the changes made by
		 * the setters */
		void updateData();
	private:
		/** Updates the transformations matrix with the current data of the
		 * RigidBody */
		void updateTransformsMatrix();

		/** Updates the Inertia Tensor in world coordinates with the current
		 * data of the RigidBody */
		void updateInertiaTensorWorld();
	};

}}

#endif		// RIGID_BODY_H
