#ifndef NORMAL_CONSTRAINT_H
#define NORMAL_CONSTRAINT_H

#include <glm/glm.hpp>
#include "../Constraint.h"

namespace fe { namespace physics {

	/**
	 * Class NormalConstraint, its used to push the given RigidBodies appart
	 * from each other
	 */
	class NormalConstraint : public Constraint
	{
	private:	// Attributes
		/** The ConstraintBounds of all the NormalConstraints */
		static constexpr ConstraintBounds kConstraintBounds = {
			0.0f,
			std::numeric_limits<float>::max()
		};

		/** The positions of the RigidBodies that will be affected by the
		 * constraint in local space */
		std::array<glm::vec3, 2> mConstraintPoints;

		/** The normal vector from the first RigidBody to the second one in
		 * local space */
		glm::vec3 mNormal;

		/** The velocity at which is going to be solved constraint */
		const float mBeta;

		/** The elapsed time since the last update */
		float mDeltaTime;

		/** The current step of the collision resolution */
		float mK;

	public:		// Functions
		/** Creates a new NormalConstraint
		 *
		 * @param	rigidBodies the two rigidBodies affected by the
		 *			Constraint
		 * @param	beta the velocity at which is going to be solved constraint
		 * @note	the movements of the RigidBodies will be restricted
		 *			relative to its origins */
		NormalConstraint(
			const std::array<RigidBody*, 2>& rigidBodies,
			float beta
		);

		/** Class destructor */
		~NormalConstraint() {};

		/** @return the value of the Bias of the constraint */
		float getBias() const override;

		/** @return the Jacobian matrix of the constraint */
		std::array<float, 12> getJacobianMatrix() const override;

		/** Sets the constraint points of the NormalConstraint
		 *
		 * @param	constraintPoints the positions of the RigidBodies that
		 *			will be affected by the constraint in local space */
		inline void setConstraintPoints(
			const std::array<glm::vec3, 2>& constraintPoints
		) { mConstraintPoints = constraintPoints; };

		/** Sets the normal vector of the NormalConstraint
		 *
		 * @param	normal the normal vector from the first RigidBody to the
		 *			second one in local space */
		inline void setConstraintNormal(const glm::vec3& normal)
		{ mNormal = normal; };

		/** Sets the elapsed time since the last update of the constraint
		 *
		 * @param	deltaTime the elapsed time since the last update in
		 *			seconds */
		inline void setDeltaTime(float deltaTime)
		{ mDeltaTime = deltaTime; };

		/** Increases the step counter */
		inline void increaseK() { ++mK; };
	};

}}

#endif		// NORMAL_CONSTRAINT_H
