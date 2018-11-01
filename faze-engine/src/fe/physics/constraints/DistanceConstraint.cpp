#include "fe/physics/constraints/DistanceConstraint.h"
#include "fe/physics/RigidBody.h"

namespace fe { namespace physics {

	constexpr ConstraintBounds DistanceConstraint::kConstraintBounds;


	DistanceConstraint::DistanceConstraint(const std::array<RigidBody*, 2>& rigidBodies) :
		Constraint(&kConstraintBounds, rigidBodies),
		mDistance(rigidBodies[1]->position - rigidBodies[0]->position) {}


	float DistanceConstraint::getBias() const
	{
		return 0.0f;
	}


	std::array<float, 12> DistanceConstraint::getJacobianMatrix() const
	{
		glm::vec3 r1xd = glm::cross(mAnchorPoints[0], mDistance);
		glm::vec3 r2xd = glm::cross(mAnchorPoints[1], mDistance);

		return {
			-mDistance.x, -mDistance.y, -mDistance.z,
			-r1xd.x, -r1xd.y, -r1xd.z,
			mDistance.x, mDistance.y, mDistance.z,
			r2xd.x, r2xd.y, r2xd.z
		};
	}


	void DistanceConstraint::setAnchorPoints(const std::array<glm::vec3, 2>& anchorPoints)
	{
		mAnchorPoints = anchorPoints;

		// Recalculate the distance vector between the RigidBodies
		glm::vec3 p1 = mRigidBodies[0]->position + mAnchorPoints[0];
		glm::vec3 p2 = mRigidBodies[1]->position + mAnchorPoints[1];
		mDistance = p2 - p1;
	}

}}
