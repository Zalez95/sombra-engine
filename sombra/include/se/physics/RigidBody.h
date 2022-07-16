#ifndef RIGID_BODY_H
#define RIGID_BODY_H

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "collision/Collider.h"
#include "forces/Force.h"

namespace se::physics {

	/**
	 * Struct RigidBodyProperties, holds all the configurable properties of a
	 * RigidBody.
	 */
	struct RigidBodyProperties
	{
		/** The different types of RigidBodies available */
		enum class Type : int
		{
			Static,				///< The RigidBody doesn't move
			Dynamic,			///< The RigidBody can move
			Count				///< The number of types
		};

		/** The type of RigidBody to use */
		Type type = Type::Static;

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
		 * be slowed down over time (only used with dynamic RigidBodies).
		 * It must be a value between 0 and 1, 0 meaning no drag at all and 1
		 * being full drag */
		float linearDrag = 0.0f;

		/** The factor by which the angular velocity of a RigidBody is going to
		 * be slowed down over time (only used with dynamic RigidBodies).
		 * It must be a value between 0 and 1, 0 meaning no drag at all and 1
		 * being full drag */
		float angularDrag = 0.0f;

		/** The friction coefficient for the friction constraints (we use the
		 * same coefficient for both static and dynamic friction) */
		float frictionCoefficient = 0.0f;

		/** The maximum motion value that the RigidBody can have before being
		 * put to Sleeping state (only used with dynamic RigidBodies) */
		float sleepMotion = 0.001f;

		/** Unused property, it can be used by the client program to store
		 * stuff */
		void* userData = nullptr;

		/** Creates a new static RigidBodyProperties */
		RigidBodyProperties() {};

		/** Creates a new dynamic RigidBodyProperties
		 *
		 * @param	mass the mass of the RigidBody
		 * @param	inertiaTensor a 3x3 matrix that stores all the moments of
		 *			inertia of the RigidBody */
		RigidBodyProperties(float mass, const glm::mat3& inertiaTensor);
	};


	/**
	 * Struct RigidBodyState, holds the position, orientation and other movement
	 * data of a RigidBody.
	 */
	struct RigidBodyState
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

		/** The value used for determining if the RigidBody should be put to a
		 * Sleeping state */
		float motion = 0.0f;

		/** The matrix that holds all the current tranformations of the
		 * RigidBody (translation and orientation) in world space */
		glm::mat4 transformsMatrix = glm::mat4(1.0f);

		/** The inverted inertia tensor of the RigidBody in world space
		 * @see		RigidBodyProperties.invertedInertiaTensor */
		glm::mat3 invertedInertiaTensorWorld = glm::mat4(1.0f);
	};


	/**
	 * Class RigidBody, it's the physics entity that can be simulated by the
	 * physics engine
	 */
	class RigidBody
	{
	public:		// Nested types
		friend class RigidBodyDynamics;
		friend class ConstraintIsland;
		using ForceSPtr = std::shared_ptr<Force>;
		using ColliderUPtr = std::unique_ptr<Collider>;

		/** The different statuses in which a RigidBody can be */
		enum class Status : int
		{
			Sleeping,			///< The simulation is stopped
			PropertiesChanged,	///< The properties have changed
			StateChanged,		///< The state has been changed by the user
			ColliderChanged,	///< The collider has changed
			ForcesChanged,		///< The forces have changed
			Count				///< The number of States
		};

	private:	// Attributes
		/** The current status of the RigidBody */
		std::bitset< static_cast<int>(Status::Count) > mStatus;

		/** The configuration properties of the RigidBody */
		RigidBodyProperties mProperties;

		/** The current movement data of the RigidBody */
		RigidBodyState mState;

		/** The collider of the RigidBody */
		ColliderUPtr mCollider;

		/** The local transforms matrix of the Collider, it's used for scaling
		 * it or adding some offset or rotation to the collider other than the
		 * one the RigidBody has. It's local in relatio to the RigidBody
		 * transforms matrix */
		glm::mat4 mColliderLocalTransforms;

		/** The forces to apply to the RigidBody */
		std::vector<ForceSPtr> mForces;

	public:		// Functions
		/** Creates a new RigidBody
		 *
		 * @param	properties the initial properties of the RigidBody
		 * @param	state the initial movement data of the RigidBody
		 * @param	collider the collider of the RigidBody
		 * @param	colliderLocalTransforms the local transforms matrix of the
		 *			Collider */
		RigidBody(
			const RigidBodyProperties& properties = RigidBodyProperties(),
			const RigidBodyState& state = RigidBodyState(),
			ColliderUPtr&& collider = nullptr,
			const glm::mat4& colliderLocalTransforms = glm::mat4(1.0f)
		);
		RigidBody(const RigidBody& other);
		RigidBody(RigidBody&& other);

		/** Class destructor */
		~RigidBody() = default;

		/** Assignment operator */
		RigidBody& operator=(const RigidBody& other);
		RigidBody& operator=(RigidBody&& other);

		/** @return	the RigidBodyProperties of the RigidBody */
		const RigidBodyProperties& getProperties() const
		{ return mProperties; };

		/** Sets RigidBodyProperties of the RigidBody
		 *
		 * @param	properties the new properties of the RigidBody
		 * @return	a reference to the current RigidBody object */
		RigidBody& setProperties(const RigidBodyProperties& properties);

		/** @return	the current RigidBodyState of the RigidBody */
		const RigidBodyState& getState() const { return mState; };

		/** Sets the RigidBodyState of the RigidBody
		 *
		 * @param	state the new movement data of the RigidBody
		 * @return	a reference to the current RigidBody object */
		RigidBody& setState(const RigidBodyState& state);

		/** @return	the Collider of the RigidBody */
		Collider* getCollider() const { return mCollider.get(); };

		/** Sets the Collider of the RigidBody
		 *
		 * @param	collider the new Collider of the RigidBody
		 * @return	a reference to the current RigidBody object
		 * @note	the parent object of the Collider will be updated to point
		 *			to the current RigidBody object */
		RigidBody& setCollider(ColliderUPtr&& collider);

		/** @return	the Collider local trasforms of the RigidBody */
		const glm::mat4& getColliderLocalTransforms() const
		{ return mColliderLocalTransforms; };

		/** Sets the local trasforms matrix of the Collider
		 *
		 * @param	localTransforms the new local transforms matrix of
		 *			the Collider
		 * @return	a reference to the current RigidBody object */
		RigidBody& setColliderLocalTrasforms(const glm::mat4& localTransforms);

		/** Adds the given Force to the RigidBody
		 *
		 * @param	force the new Force
		 * @return	the current RigidBody object */
		RigidBody& addForce(const ForceSPtr& force);

		/** Iterates through all the Forces of the RigidBody calling the given
		 * callback function
		 *
		 * @param	callback the function to call for each Force */
		template <typename F>
		void processForces(F&& callback) const;

		/** Removes the given Force from the RigidBody
		 *
		 * @param	force the Force to remove
		 * @return	the current RigidBody object */
		RigidBody& removeForce(const ForceSPtr& force);

		/** Checks if the RigidBody is in the given Status
		 *
		 * @param	status the Status to check
		 * @return	true if the RigidBody is in the given state, false
		 *			otherwise */
		bool getStatus(Status status) const
		{ return mStatus[static_cast<int>(status)]; };

		/** Sets the RigidBody's status to the given value
		 *
		 * @param	status the status to set
		 * @param	value the new value of the status */
		void setStatus(Status status, bool value);

		/** Updates the RigidBody's transform matrix and inertia tensor in
		 * world coordinates with the changes made to the RigidBody's position
		 * and orientation. It also updates it's Collider trasforms if it has
		 * one */
		void updateTransforms();

		/** Updates the motion value of the RigidBody. The motion value
		 * is calculated as a recency-weighted average of its current value and
		 * its old ones
		 *
		 * @param	bias the proportion of the new motion value due to its old
		 *			value
		 * @param	maxMotion the maximum motion value */
		void updateMotion(float bias, float maxMotion);
	};


	template <typename F>
	void RigidBody::processForces(F&& callback) const
	{
		for (auto& force : mForces) {
			callback(force);
		}
	}

}

#endif		// RIGID_BODY_H
