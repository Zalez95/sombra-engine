#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include <array>

namespace se::physics {

	class RigidBody;


	/**
	 * Struct ConstraintBounds. It defines the bounds of the lambda variable of
	 * a Constraint.
	 */
	struct ConstraintBounds
	{
		/** The minimum value that the alpha variable of the constraint could
		 * have */
		float alphaMin;

		/** The maximum value that the alpha variable of the constraint could
		 * have */
		float alphaMax;
	};


	/**
	 * Class Constraint, a physics constraint is a parameter that the movement
	 * of one or two RigidBodies must obey. This allows us to limit the movement
	 * between RigidBodies.
	 *
	 * @note	the constraints can only restrict the movement between two
	 *			RigidBodies
	 */
	class Constraint
	{
	protected:	// Attributes
		/** A pointer to the bounds of the Constraint */
		const ConstraintBounds* mConstraintBounds;

		/** The two RigidBodies affected by the constraint */
		std::array<RigidBody*, 2> mRigidBodies;

	public:		// Functions
		/** Creates a new Constraint */
		Constraint() :
			mConstraintBounds(nullptr), mRigidBodies{ nullptr, nullptr } {};

		/** Creates a new Constraint
		 *
		 * @param	constraintBounds a pointer to the data of the Constraint
		 * @param	rigidBodies the two rigidBodies affected by the
		 *			Constraint */
		Constraint(
			const ConstraintBounds* constraintBounds,
			const std::array<RigidBody*, 2>& rigidBodies
		) : mConstraintBounds(constraintBounds),
			mRigidBodies(rigidBodies) {};

		/** Class destructor */
		virtual ~Constraint() = default;

		/** @return the a pointer to the ConstraintBounds of the Constraint */
		const ConstraintBounds* getConstraintBounds() const
		{ return mConstraintBounds; };

		/** Returns the specified RigidBody
		 *
		 * @param	rb the index of the RigidBody (0 or 1)
		 * @return	a pointer the the RigidBody */
		RigidBody* getRigidBody(std::size_t rb) const
		{ return mRigidBodies[rb]; };

		/** @return the value of the Bias of the constraint */
		virtual float getBias() const = 0;

		/** @return the Jacobian matrix of the constraint */
		virtual std::array<float, 12> getJacobianMatrix() const = 0;
	};

}

#endif		// CONSTRAINT_H
