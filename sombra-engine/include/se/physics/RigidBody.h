#ifndef RIGID_BODY_H
#define RIGID_BODY_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace se::physics {

	/**
	 * Struct RigidBody, a Rigid Body holds the position, orientation and other
	 * movement data of any object in the physics system.
	 */
	struct RigidBody
	{
		/** The inverse of the mass
		 * @note	We store the mass inverted because it's more useful for
		 *			storing object with infinite mass */
		float invertedMass;

		/** The inertia tensor of the RigidBody. It is a 3x3 matrix in
		 * Body Space that stores all the moments of inertia of a
		 * RigidBody.
		 * @note	As with the mass, the inertia tensor is stored inverted
		 *			so we don't need to inverse the matrix in later calculus. */
		glm::mat3 invertedInertiaTensor;

		/** The inertia tensor in world space
		 * @see		mInvertedInertiaTensor
		 * @note	is a mat3 because it doesn't matter the location, only
		 *			the orientation */
		glm::mat3 invertedInertiaTensorWorld;

		/** The factor by the linear velocity of the RigidBody is going to
		 * be slowed down each time the integrate method is called so the
		 * RigidBody doesn't move forever */
		float linearSlowDown;

		/** The factor by the angular velocity of the RigidBody is going to
		 * be slowed down each time the integrate method is called so the
		 * RigidBody doesn't move forever */
		float angularSlowDown;

		/** The linear position of the origin (center of mass) of the
		 * RigidBody in world space */
		glm::vec3 position;

		/** The orientation of the RigidBody in world space */
		glm::quat orientation;

		/** The linear velocity of the RigidBody in world space */
		glm::vec3 linearVelocity;

		/** The angular velocity of the RigidBody in world space represented as
		 * a scaled vector, where the magnitude is the rotation rate around the
		 * normalized axis */
		glm::vec3 angularVelocity;

		/** The linear acceleration of the RigidBody in world space */
		glm::vec3 linearAcceleration;

		/** The angular acceleration of the RigidBody in world space */
		glm::vec3 angularAcceleration;

		/** A vector with the sum of all the forces currently applied to
		 * the RigidBody */
		glm::vec3 forceSum;

		/** A vector with the sum of all the torques currently applied to
		 * the RigidBody */
		glm::vec3 torqueSum;

		/** The matrix that holds all the current tranformations of the
		 * RigidBody (translation and orientation) in world space to avoid
		 * recalculating the position and orientation */
		glm::mat4 transformsMatrix;

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
	};


	/** Updates the given RigidBody's transform matrix and inertia tensor in
	 * world coordinates with the changes made to its position and orientation
	 *
	 * @param	rigidBody the RigidBody to update */
	void updateRigidBodyData(RigidBody& rigidBody);

}

#endif		// RIGID_BODY_H
