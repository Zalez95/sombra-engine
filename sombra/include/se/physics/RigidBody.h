#ifndef RIGID_BODY_H
#define RIGID_BODY_H

#include <bitset>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "forces/Force.h"

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
		float invertedMass = 0.0f;

		/** The inertia tensor of the RigidBody. It's a 3x3 matrix in local
		 * space that stores all the moments of inertia of the RigidBody.
		 * @note	is a mat3 because it doesn't matter the location, only
		 *			the orientation. The inertia tensor is stored inverted
		 *			so we don't need to inverse the matrix in later calculus. */
		glm::mat3 invertedInertiaTensor = glm::mat3(0.0f);

		/** The factor by which the linear velocity of the RigidBody is going to
		 * be slowed down over time */
		float linearDrag = 0.0f;

		/** The factor by which the angular velocity of a RigidBody is going to
		 * be slowed down over time */
		float angularDrag = 0.0f;

		/** The friction coefficient for the friction constraints (we use the
		 * same coefficient for both static and dynamic friction) */
		float frictionCoefficient = 0.0f;

		/** The maximum motion value that the RigidBody can have before being
		 * put to Sleeping state */
		float sleepMotion = 0.001f;

		/** Creates a new RigidBodyConfig with infinite mass */
		RigidBodyConfig() {};

		/** Creates a new RigidBodyConfig
		 *
		 * @param	mass the mass of the RigidBody
		 * @param	inertiaTensor a 3x3 matrix that stores all the moments of
		 *			inertia of the RigidBody */
		RigidBodyConfig(
			float mass, const glm::mat3& inertiaTensor
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
		glm::vec3 position = glm::vec3(0.0f);

		/** The orientation of the RigidBody in world space */
		glm::quat orientation = glm::quat(1.0f, glm::vec3(0.0f));

		/** The linear velocity of the RigidBody in world space */
		glm::vec3 linearVelocity = glm::vec3(0.0f);

		/** The angular velocity of the RigidBody in world space represented as
		 * a scaled vector, where the magnitude is the rotation rate around the
		 * normalized axis */
		glm::vec3 angularVelocity = glm::vec3(0.0f);

		/** The linear acceleration of the RigidBody in world space */
		glm::vec3 linearAcceleration = glm::vec3(0.0f);

		/** The angular acceleration of the RigidBody in world space */
		glm::vec3 angularAcceleration = glm::vec3(0.0f);

		/** A vector with the sum of all the forces currently applied to
		 * the RigidBody */
		glm::vec3 forceSum = glm::vec3(0.0f);

		/** A vector with the sum of all the torques currently applied to
		 * the RigidBody */
		glm::vec3 torqueSum = glm::vec3(0.0f);
	};


	/**
	 * Class RigidBody
	 */
	class RigidBody
	{
	private:	// Nested types
		using ForceSPtr = std::shared_ptr<Force>;
		friend class RigidBodyDynamics;

	private:	// Attributes
		/** The current state of the RigidBody */
		std::bitset< static_cast<int>(RigidBodyState::Count) > mState;

		/** The initial configuration properties of the RigidBody */
		RigidBodyConfig mConfig;

		/** The current movement data of the RigidBody */
		RigidBodyData mData;

		/** The forces to apply to the RigidBody */
		std::vector<ForceSPtr> mForces;

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
			const RigidBodyConfig& config = RigidBodyConfig(),
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

		/** @return	the RigidBodyConfig of the RigidBody */
		RigidBodyConfig& getConfig() { return mConfig; };

		/** @return	the current RigidBodyData of the RigidBody */
		const RigidBodyData& getData() const { return mData; };

		/** @return	the current RigidBodyData of the RigidBody */
		RigidBodyData& getData() { return mData; };

		/** Adds the given Force to the RigidBody
		 *
		 * @param	force the new Force
		 * @return	the current RigidBody object */
		RigidBody& addForce(ForceSPtr force);

		/** Iterates through all the Forces of the RigidBody calling the given
		 * callback function
		 *
		 * @param	callback the function to call for each Force */
		template <typename F>
		void processForces(F callback) const;

		/** Removes the given Force from the RigidBody
		 *
		 * @param	force the Force to remove
		 * @return	the current RigidBody object */
		RigidBody& removeForce(ForceSPtr force);

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


	template <typename F>
	void RigidBody::processForces(F callback) const
	{
		for (auto& force : mForces) {
			callback(force);
		}
	}

}

#endif		// RIGID_BODY_H
