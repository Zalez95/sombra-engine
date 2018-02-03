#ifndef PHYSICS_ENTITY_H
#define PHYSICS_ENTITY_H

#include <memory>
#include "RigidBody.h"
#include "../collision/Collider.h"

namespace fe { namespace physics {

	/**
	 * Class PhysicsEntity,
	 */
	class PhysicsEntity
	{
	private:	// Attributes
		/** The RigidBody of the PhysicsEntity */
		RigidBody mRigidBody;

		/** The Collider of the PhysicsEntity */
		std::unique_ptr<collision::Collider> mCollider;

		/** The matrix that holds the offset of the Collider relative to the
		 * RigidBody of the PhysicsEntity */
		glm::mat4 mColliderOffset;

	public:		// Functions
		/** Creates a new PhysicsEntity
		 *
		 * @param	rigidBody the RigidBody of the PhysicsEntity */
		PhysicsEntity(const RigidBody& rigidBody) :
			mRigidBody(rigidBody), mColliderOffset(1.0f) {};

		/** Creates a new PhysicsEntity with a Collider
		 *
		 * @param	rigidBody the RigidBody of the PhysicsEntity
		 * @param	collider a pointer to the Collider of the PhysicsEntity
		 * @param	colliderOffset the offset matrix of the Collider relative
		 *			to the rigidBody of the PhysicsEntity */
		PhysicsEntity(
			const RigidBody& rigidBody,
			std::unique_ptr<collision::Collider> collider,
			const glm::mat4& colliderOffset
		) : mRigidBody(rigidBody),
			mCollider(std::move(collider)),
			mColliderOffset(colliderOffset) {};

		/** Class destructor */
		~PhysicsEntity() {};

		/** @return	a pointer to the RigidBody of the PhysicsEntity */
		inline RigidBody* getRigidBody()
		{ return &mRigidBody; };

		/** @return	a pointer to the Collider of the PhysicsEntity */
		inline collision::Collider* getCollider()
		{ return mCollider.get(); };

		/** @return	the offset of the Collider from the Particle / RigidBody
		 * of the PhysicsEntity */
		inline glm::mat4 getColliderOffset() const
		{ return mColliderOffset; };
	};

}}

#endif		// PHYSICS_ENTITY_H
