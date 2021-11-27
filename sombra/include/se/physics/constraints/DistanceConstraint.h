#ifndef DISTANCE_CONSTRAINT_H
#define DISTANCE_CONSTRAINT_H

#include <glm/glm.hpp>
#include "Constraint.h"

namespace se::physics {

	/**
	 * Class DistanceConstraint, its used to maintain the given RigidBodies at
	 * the desired distance between each other
	 */
	class DistanceConstraint : public Constraint
	{
	private:	// Attributes
		/** The ConstraintBounds of all the DistanceConstraints */
		static constexpr ConstraintBounds kConstraintBounds = {
			-std::numeric_limits<float>::max(),
			std::numeric_limits<float>::max()
		};

		/** The positions of the RigidBodies that will be affected by the
		 * constraint in local space */
		std::array<glm::vec3, 2> mAnchorPoints;

		/** If the DistanceConstraint has been updated or not */
		bool mUpdated;

	public:		// Functions
		/** Creates a new DistanceConstraint
		 *
		 * @param	rigidBodies the two rigidBodies affected by the
		 *			Constraint
		 * @note	initially the anchor points are located in the RigidBody
		 *			origins */
		DistanceConstraint(const std::array<RigidBody*, 2>& rigidBodies) :
			Constraint(rigidBodies),
			mAnchorPoints{ glm::vec3(0.0f), glm::vec3(0.0f) },
			mUpdated(true) {};

		/** @copydoc Constraint::clone() */
		virtual std::unique_ptr<Constraint> clone() const override
		{ return std::make_unique<DistanceConstraint>(*this); };

		/** @return	the ConstraintBounds of the Constraint */
		virtual const ConstraintBounds& getConstraintBounds() const override
		{ return kConstraintBounds; };

		/** @return	the value of the Bias of the constraint */
		virtual float getBias() const override { return 0.0f; };

		/** @return	the Jacobian matrix of the constraint */
		virtual std::array<float, 12> getJacobianMatrix() const override;

		/** @copydoc Constraint::updated() */
		bool updated() const override { return mUpdated; };

		/** @copydoc Constraint::resetUpdatedState() */
		void resetUpdatedState() override { mUpdated = false; };

		/** Sets the anchor points of the DistanceConstraint
		 *
		 * @param	anchorPoints the positions of the RigidBodies that
		 *			will be affected by the constraint in local space */
		void setAnchorPoints(const std::array<glm::vec3, 2>& anchorPoints);

		/** @return	the anchor points of the DistanceConstraint */
		const std::array<glm::vec3, 2>& getAnchorPoints() const
		{ return mAnchorPoints; };
	};

}

#endif		// DISTANCE_CONSTRAINT_H
