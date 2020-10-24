#ifndef RIGID_BODY_H
#define RIGID_BODY_H

#include <bitset>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace se::physics {

	/** The different states in which a RigidBody can be */
	enum class RigidBodyState : int
	{
		Sleeping,			///< The RigidBody simulation is stopped
		Integrated,			///< Changed due to the dynamics simulation
		ConstraintsSolved,	///< Changed due to the constraints
		Count				///< The number of States
	};


	/**
	 * Struct RigidBodyConfig, holds all the constant properties of a RigidBody.
	 */
	struct RigidBodyConfig
	{
		/** The inverse of the mass of the RigidBody
		 * @note	We store the mass inverted because it's more useful for
		 *			storing RigidBodies with infinite mass */
		float invertedMass;

		/** The inertia tensor of the RigidBody. It's a 3x3 matrix in local
		 * space that stores all the moments of inertia of the RigidBody.
		 * @note	is a mat3 because it doesn't matter the location, only
		 *			the orientation. The inertia tensor is stored inverted
		 *			so we don't need to inverse the matrix in later calculus. */
		glm::mat3 invertedInertiaTensor;

		/** The factor by which the linear velocity of the RigidBody is going to
		 * be slowed down over time */
		float linearDrag;

		/** The factor by which the angular velocity of a RigidBody is going to
		 * be slowed down over time */
		float angularDrag;

		/** The friction coefficient for the friction constraints (we use the
		 * same coefficient for both static and dynamic friction) */
		float frictionCoefficient;

		/** The maximum motion value that the RigidBody can have before being
		 * put to Sleeping state */
		float sleepMotion;

		/** Creates a new RigidBodyConfig with infinite mass
		 *
		 * @param	sleepMotion the maximum motion value that the RigidBody can
		 *			have before being put to Sleeping state */
		RigidBodyConfig(float sleepMotion);

		/** Creates a new RigidBodyConfig
		 *
		 * @param	mass the mass of the RigidBody
		 * @param	inertiaTensor a 3x3 matrix that stores all the moments of
		 *			inertia of the RigidBody
		 * @param	sleepMotion the maximum motion value that the RigidBody can
		 *			have before being put to Sleeping state */
		RigidBodyConfig(
			float mass, const glm::mat3& inertiaTensor, float sleepMotion
		);
	};


	/**
	 * Struct RigidBodyData, holds the position, orientation and other movement
	 * data of a RigidBody.
	 */
	struct RigidBodyData
	{
		/** The linear position of the origin (center of mass) of the RigidBody
		 * in world space */
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

		/** Creates a new RigidBodyData */
		RigidBodyData();
	};


	/**
	 * Class RigidBody
	 */
	class RigidBody
	{
	private:	// Nested types
		friend class RigidBodyDynamics;

	private:	// Attributes
		/** The current state of the RigidBody */
		std::bitset< static_cast<int>(RigidBodyState::Count) > mState;

		/** The initial configuration properties of the RigidBody */
		const RigidBodyConfig mConfig;

		/** The current movement data of the RigidBody */
		RigidBodyData mData;

		/** The matrix that holds all the current tranformations of the
		 * RigidBody (translation and orientation) in world space */
		glm::mat4 mTransformsMatrix;

		/** The inertia tensor of the RigidBody in world space
		 * @see		mInvertedInertiaTensor */
		glm::mat3 mInvertedInertiaTensorWorld;

		/** The value used for determining if the RigidBody should be put to a
		 * Sleeping state */
		float mMotion;

	public:		// Functions
		/** Creates a new RigidBody
		 *
		 * @param	config the configuration data of the RigidBody
		 * @param	data the initial movement data of the RigidBody */
		RigidBody(
			const RigidBodyConfig& config,
			const RigidBodyData& data = RigidBodyData()
		);

		/** Check if the RigidBody is in the given state
		 *
		 * @param	state the state to check
		 * @return	true if the RigidBody is in the given state, false
		 *			otherwise */
		bool checkState(RigidBodyState state) const
		{ return mState[static_cast<int>(state)]; };

		/** @return	the RigidBodyConfig of the RigidBody */
		const RigidBodyConfig& getConfig() const { return mConfig; };

		/** @return	the current RigidBodyData of the RigidBody */
		const RigidBodyData& getData() const { return mData; };

		/** @return	the current RigidBodyData of the RigidBody */
		RigidBodyData& getData() { return mData; };

		/** @return	the current transformation matrix of the RigidBody */
		const glm::mat4& getTransformsMatrix() const
		{ return mTransformsMatrix; };

		/** @return	the inverse of the intertia tensor matrix of the RigidBody
		 *			in world space */
		const glm::mat3& getInvertedInertiaTensorWorld() const
		{ return mInvertedInertiaTensorWorld; };

		/** @return	the motion value of the RigidBody */
		float getMotion() const { return mMotion; };

		/** Synchronizes the internal state of the RigidBody with the changes
		 * made to its data struct.
		 *
		 * @note	this function must be called each time the RigidBody's
		 *			RigidBodyData is changed */
		void synchWithData();
	};

}

#endif		// RIGID_BODY_H
