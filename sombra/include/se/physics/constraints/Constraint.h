#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include <array>
#include <memory>

namespace se::physics {

	class RigidBody;


	/**
	 * Struct ConstraintBounds. It defines the bounds of the lambda variable of
	 * a Constraint.
	 */
	struct ConstraintBounds
	{
		/** The minimum value that the lambda variable of the constraint could
		 * have */
		float lambdaMin;

		/** The maximum value that the lambda variable of the constraint could
		 * have */
		float lambdaMax;
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
		/** The two RigidBodies affected by the constraint */
		std::array<RigidBody*, 2> mRigidBodies;

	public:		// Functions
		/** Creates a new Constraint */
		Constraint() : mRigidBodies{ nullptr, nullptr } {};

		/** Creates a new Constraint
		 *
		 * @param	rigidBodies the two rigidBodies affected by the
		 *			Constraint */
		Constraint(const std::array<RigidBody*, 2>& rigidBodies) :
			mRigidBodies(rigidBodies) {};

		/** Class destructor */
		virtual ~Constraint() = default;

		/** Returns the specified RigidBody
		 *
		 * @param	rb the index of the RigidBody (0 or 1)
		 * @return	a pointer the the RigidBody */
		RigidBody* getRigidBody(std::size_t rb) const
		{ return mRigidBodies[rb]; };

		/** @return	a pointer to a copy of the current Constraint */
		virtual std::unique_ptr<Constraint> clone() const = 0;

		/** @return	the ConstraintBounds of the Constraint */
		virtual const ConstraintBounds& getConstraintBounds() const = 0;

		/** @return	the value of the Bias of the constraint */
		virtual float getBias() const = 0;

		/** @return	the Jacobian matrix of the constraint */
		virtual std::array<float, 12> getJacobianMatrix() const = 0;
	};

}

#endif		// CONSTRAINT_H
