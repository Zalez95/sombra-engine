#ifndef RIGID_BODY_DYNAMICS_H
#define RIGID_BODY_DYNAMICS_H

namespace fe { namespace physics {

	struct RigidBody;


	/** Integrates the position and orientation of the given RigidBody, with
	 * its velocities, accelerations and forces, by the given amount of time.
	 *
	 * @param	rigidBody the RigidBody to update
	 * @param	deltaTime the difference time used to integrate the
	 *			attributes of the RigidBody in seconds */
	void integrate(RigidBody& rigidBody, float deltaTime);


	/** Updates the linear acceleration of the given RigidBody with the sum
	 * of all the forces applied to it
	 *
	 * @param	rigidBody the RigidBody to update */
	void applyForces(RigidBody& rigidBody);


	/** Updates the angular acceleration of the given RigidBody with the sum
	 * of all the torques applied to it
	 *
	 * @param	rigidBody the RigidBody to update */
	void applyTorques(RigidBody& rigidBody);


	/** Integrates the given RigidBody's linear acceleration to calculate
	 * its new linear velocity
	 *
	 * @param	rigidBody the RigidBody to update
	 * @param	deltaTime the difference of time used to integrate the
	 *			linear acceleration of the RigidBody in seconds */
	void integrateLinearAcceleration(RigidBody& rigidBody, float deltaTime);


	/** Integrates the given RigidBody's angular acceleration to calculate
	 * its new angular velocity
	 *
	 * @param	rigidBody the RigidBody to update
	 * @param	deltaTime the difference of time used to integrate the
	 *			angular acceleration of the RigidBody in seconds */
	void integrateAngularAcceleration(RigidBody& rigidBody, float deltaTime);


	/** Integrates the given RigidBody's linear velocity to calculate its
	 * new position
	 *
	 * @param	rigidBody the RigidBody to update
	 * @param	deltaTime the difference of time used to integrate the
	 *			linear velocity of the RigidBody in seconds */
	void integrateLinearVelocity(RigidBody& rigidBody, float deltaTime);


	/** Integrates the given RigidBody's angular velocity to calculate its
	 * new orientation
	 *
	 * @param	rigidBody the RigidBody to update
	 * @param	deltaTime the difference of time used to integrate the
	 *			angular velocity of the RigidBody in seconds */
	void integrateAngularVelocity(RigidBody& rigidBody, float deltaTime);

}}

#endif		// RIGID_BODY_DYNAMICS_H