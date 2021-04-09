#ifndef RIGID_BODY_DYNAMICS_H
#define RIGID_BODY_DYNAMICS_H

namespace se::physics {

	class RigidBody;


	/**
	 * Class RigidBodyDynamics, holds all the functions used by the
	 * PhysicsEngine to interact with the RigidBodies.
	 */
	class RigidBodyDynamics
	{
	public:		// Functions
		/** Processes the Forces stored in the given RigidBody and calculates
		 * its sum
		 *
		 * @param	rigidBody the RigidBody to update */
		static void processForces(RigidBody& rigidBody);

		/** Integrates the position and orientation of the given RigidBody, with
		 * its velocities, accelerations and forces, by the given amount of time
		 *
		 * @param	rigidBody the RigidBody to update
		 * @param	deltaTime the difference time used to integrate the
		 *			attributes of the RigidBody in seconds */
		static void integrate(RigidBody& rigidBody, float deltaTime);

		/** Updates the linear acceleration of the given RigidBody with the sum
		 * of all the forces applied to it
		 *
		 * @param	rigidBody the RigidBody to update */
		static void applyForces(RigidBody& rigidBody);

		/** Updates the angular acceleration of the given RigidBody with the sum
		 * of all the torques applied to it
		 *
		 * @param	rigidBody the RigidBody to update */
		static void applyTorques(RigidBody& rigidBody);

		/** Integrates the given RigidBody's linear acceleration to calculate
		 * its new linear velocity
		 *
		 * @param	rigidBody the RigidBody to update
		 * @param	deltaTime the difference of time used to integrate the
		 *			linear acceleration of the RigidBody in seconds */
		static void integrateLinearAcceleration(
			RigidBody& rigidBody, float deltaTime
		);

		/** Integrates the given RigidBody's angular acceleration to calculate
		 * its new angular velocity
		 *
		 * @param	rigidBody the RigidBody to update
		 * @param	deltaTime the difference of time used to integrate the
		 *			angular acceleration of the RigidBody in seconds */
		static void integrateAngularAcceleration(
			RigidBody& rigidBody, float deltaTime
		);

		/** Integrates the given RigidBody's linear velocity to calculate its
		 * new position
		 *
		 * @param	rigidBody the RigidBody to update
		 * @param	deltaTime the difference of time used to integrate the
		 *			linear velocity of the RigidBody in seconds */
		static void integrateLinearVelocity(
			RigidBody& rigidBody, float deltaTime
		);

		/** Integrates the given RigidBody's angular velocity to calculate its
		 * new orientation
		 *
		 * @param	rigidBody the RigidBody to update
		 * @param	deltaTime the difference of time used to integrate the
		 *			angular velocity of the RigidBody in seconds */
		static void integrateAngularVelocity(
			RigidBody& rigidBody, float deltaTime
		);

		/** Updates the given RigidBody's transform matrix and inertia tensor in
		 * world coordinates with the changes made to the RigidBody's position
		 * and orientation
		 *
		 * @param	rigidBody the RigidBody to update */
		static void updateTransformsMatrix(RigidBody& rigidBody);

		/** Updates the motion value of the given RigidBody. The motion value
		 * is calculated as a recency-weighted average of its current value and
		 * its old ones
		 *
		 * @param	rigidBody the RigidBody to update
		 * @param	bias the proportion of the new motion value due to its old
		 *			value
		 * @param	maxMotion the maximum motion value */
		static void updateMotion(
			RigidBody& rigidBody,
			float bias, float maxMotion
		);

		/** Sets the RigidBody's state to the given value
		 *
		 * @param	rigidBody the RigidBody to update
		 * @param	state the state to set
		 * @param	value the new value of the state */
		static void setState(
			RigidBody& rigidBody,
			RigidBodyState state, bool value
		);
	};

}

#endif		// RIGID_BODY_DYNAMICS_H
