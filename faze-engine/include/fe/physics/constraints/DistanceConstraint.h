#ifndef DISTANCE_CONSTRAINT_H
#define DISTANCE_CONSTRAINT_H

#include <glm/glm.hpp>
#include "../Constraint.h"

namespace fe { namespace physics {

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

		/** The desired distance between the RigidBodies */
		glm::vec3 mDistance;

	public:		// Functions
		/** Creates a new DistanceConstraint
		 *
		 * @param	rigidBodies the two rigidBodies affected by the
		 *			Constraint
		 * @note	the movements of the RigidBodies will be restricted
		 *			relative to its origins */
		DistanceConstraint(const std::array<RigidBody*, 2>& rigidBodies);

		/** Class destructor */
		~DistanceConstraint() {};

		/** @return the value of the Bias of the constraint */
		float getBias() const override;

		/** @return the Jacobian matrix of the constraint */
		std::array<float, 12> getJacobianMatrix() const override;

		/** Sets the anchor points of the DistanceConstraint
		 *
		 * @param	anchorPoints the positions of the RigidBodies that
		 *			will be affected by the constraint in local space */
		void setAnchorPoints(const std::array<glm::vec3, 2>& anchorPoints);
	};

}}

#endif		// DISTANCE_CONSTRAINT_H
