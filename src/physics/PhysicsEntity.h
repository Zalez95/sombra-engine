#ifndef PHYSICS_ENTITY_H
#define PHYSICS_ENTITY_H

#include <memory>
#include "Particle.h"
#include "RigidBody.h"
#include "collision/Collider.h"

namespace physics {

    /**
     * PhysicsEntityType tells us if the physics of the PhysicsEntity must be
     * calculated as a Particle (linear movement) or as a RigidBody
     * (linear and rotational movement)
     */
    enum PhysicsEntityType
    {
        PARTICLE,
        RIGID_BODY
    };


	/**
	 * Class PhysicsEntity
	 */
	class PhysicsEntity
	{
	private:	// Attributes
        /** The type of the PhysicsEntity */
        PhysicsEntityType mType;

		/** The RigidBody of the PhysicsEntity */
		std::unique_ptr<RigidBody> mRigidBody;

		/** The Particle of the PhysicsEntity */
		std::unique_ptr<Particle> mParticle;

		/** The Collider of the PhysicsEntity */
		std::unique_ptr<Collider> mCollider;

		/** The matrix that holds the offset of the Collider from the
		 * Particle / RigidBody of the PhysicsEntity */
		glm::mat4 mColliderOffset;

	public:	    // Functions
		/** Creates a new PhysicsEntity of Particle type */
		PhysicsEntity(
			std::unique_ptr<Particle> particle,
			std::unique_ptr<Collider> collider,
			const glm::mat4& colliderOffset
		) : mType(PhysicsEntityType::PARTICLE),
            mParticle(std::move(particle)),
			mCollider(std::move(collider)),
			mColliderOffset(colliderOffset) {};

		/** Creates a new PhysicsEntity of RigidBody type */
		PhysicsEntity(
            std::unique_ptr<RigidBody> rigidBody,
			std::unique_ptr<Collider> collider,
			const glm::mat4& colliderOffset
		) : mType(PhysicsEntityType::RIGID_BODY),
            mRigidBody(std::move(rigidBody)),
			mCollider(std::move(collider)),
			mColliderOffset(colliderOffset) {};

		/** Class destructor */
		~PhysicsEntity() {};

		/** @return	the type of the PhysicsEntity */
		inline PhysicsEntityType getType() const
		{ return mType; };

		/** @return	a pointer to the Particle of the PhysicsEntity */
		inline Particle* getParticle() const
		{ return mParticle.get(); };

		/** @return	a pointer to the RigidBody of the PhysicsEntity */
		inline RigidBody* getRigidBody() const
		{ return mRigidBody.get(); };

		/** @return	a pointer to the Collider of the PhysicsEntity */
		inline Collider* getCollider() const
		{ return mCollider.get(); };

		/** @return	the offset of the Collider from the Particle / RigidBody
		 * of the PhysicsEntity */
		inline glm::mat4 getColliderOffset() const
		{ return mColliderOffset; };
	};

}

#endif		// PHYSICS_ENTITY_H
