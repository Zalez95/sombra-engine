#include <cassert>
#include <algorithm>
#include "se/physics/RigidBody.h"
#include "RigidBodyDynamics.h"

namespace se::physics {

	RigidBodyProperties::RigidBodyProperties(float mass, const glm::mat3& inertiaTensor) :
		type(Type::Dynamic)
	{
		assert(mass > 0.0f && "The mass must be greater than zero");

		invertedMass			= 1.0f / mass;
		invertedInertiaTensor	= glm::inverse(inertiaTensor);
	}


	RigidBody::RigidBody(
		const RigidBodyProperties& properties, const RigidBodyState& state,
		ColliderUPtr&& collider, const glm::mat4& colliderLocalTransforms
	) : mProperties(properties), mState(state),
		mCollider(std::move(collider)), mColliderLocalTransforms(colliderLocalTransforms)
	{
		setStatus(RigidBodyState::Status::UpdatedByUser, true);
		updateTransforms();
	}


	RigidBody::RigidBody(const RigidBody& other) :
		mProperties(other.mProperties), mState(other.mState),
		mColliderLocalTransforms(other.mColliderLocalTransforms), mForces(other.mForces)
	{
		if (other.mCollider) {
			mCollider = other.mCollider->clone();
			mCollider->setParent(this);
		}
	}


	RigidBody::RigidBody(RigidBody&& other) :
		mProperties(std::move(other.mProperties)), mState(std::move(other.mState)),
		mCollider(std::move(other.mCollider)), mColliderLocalTransforms(std::move(other.mColliderLocalTransforms)),
		mForces(std::move(other.mForces))
	{
		if (mCollider) {
			mCollider->setParent(this);
		}
	}


	RigidBody& RigidBody::operator=(const RigidBody& other)
	{
		mProperties = other.mProperties;
		mState = other.mState;
		if (other.mCollider) {
			mCollider = other.mCollider->clone();
			mCollider->setParent(this);
		}
		mColliderLocalTransforms = other.mColliderLocalTransforms;
		mForces = other.mForces;

		return *this;
	}


	RigidBody& RigidBody::operator=(RigidBody&& other)
	{
		mProperties = std::move(other.mProperties);
		mState = std::move(other.mState);
		mCollider = std::move(other.mCollider);
		mColliderLocalTransforms = std::move(other.mColliderLocalTransforms);
		mForces = std::move(other.mForces);

		if (mCollider) {
			mCollider->setParent(this);
		}

		return *this;
	}


	RigidBody& RigidBody::setProperties(const RigidBodyProperties& properties)
	{
		mProperties = properties;
		setStatus(RigidBodyState::Status::UpdatedByUser, true);
		return *this;
	}


	RigidBody& RigidBody::setState(const RigidBodyState& state)
	{
		mState = state;
		setStatus(RigidBodyState::Status::UpdatedByUser, true);
		updateTransforms();
		return *this;
	}


	RigidBody& RigidBody::setCollider(ColliderUPtr&& collider)
	{
		mCollider = std::move(collider);
		if (mCollider) {
			mCollider->setParent(this);
			mCollider->setTransforms(mState.transformsMatrix * mColliderLocalTransforms);
		}
		setStatus(RigidBodyState::Status::ColliderChanged, true);
		return *this;
	}


	RigidBody& RigidBody::setColliderLocalTrasforms(const glm::mat4& localTransforms)
	{
		mColliderLocalTransforms = localTransforms;
		if (mCollider) {
			mCollider->setTransforms(mState.transformsMatrix * mColliderLocalTransforms);
		}
		setStatus(RigidBodyState::Status::ColliderChanged, true);
		return *this;
	}


	RigidBody& RigidBody::addForce(ForceSPtr force)
	{
		mForces.emplace_back(std::move(force));
		setStatus(RigidBodyState::Status::UpdatedByUser, true);
		return *this;
	}


	RigidBody& RigidBody::removeForce(ForceSPtr force)
	{
		mForces.erase(std::remove(mForces.begin(), mForces.end(), force), mForces.end());
		setStatus(RigidBodyState::Status::UpdatedByUser, true);
		return *this;
	}


	void RigidBody::setStatus(RigidBodyState::Status status, bool value)
	{
		if (value) {
			mState.status.set( static_cast<int>(status) );

			// Set the motion of the RigidBody to a larger value than
			// mSleepEpsilon to prevent it from falling sleep instantly
			if (status == RigidBodyState::Status::Sleeping) {
				mState.motion = 2.0f * mProperties.sleepMotion;
			}
		}
		else {
			mState.status.reset( static_cast<int>(status) );

			if (status == RigidBodyState::Status::Sleeping) {
				mState.motion = 0.0f;
			}
		}
	}


	void RigidBody::updateTransforms()
	{
		// Update the transforms matrix of the RigidBody
		glm::mat4 translation	= glm::translate(glm::mat4(1.0f), mState.position);
		glm::mat4 rotation		= glm::mat4_cast(mState.orientation);
		mState.transformsMatrix = translation * rotation;

		// Update the inertia tensor of the RigidBody
		glm::mat3 inverseTransformsMat3 = glm::inverse(mState.transformsMatrix);
		mState.invertedInertiaTensorWorld = glm::transpose(inverseTransformsMat3)
			* mProperties.invertedInertiaTensor
			* inverseTransformsMat3;

		// Update the trasforms of the Collider
		if (mCollider) {
			mCollider->setTransforms(mState.transformsMatrix * mColliderLocalTransforms);
		}
	}


	void RigidBody::updateMotion(float bias, float maxMotion)
	{
		float motion = glm::dot(mState.linearVelocity, mState.linearVelocity)
			+ glm::dot(mState.angularVelocity, mState.angularVelocity);
		motion = bias * mState.motion + (1.0f - bias) * motion;
		mState.motion = std::min(motion, maxMotion);
	}

}
