#include "se/physics/constraints/DistanceConstraint.h"
#include "se/physics/RigidBody.h"

namespace se::physics {

	std::array<float, 12> DistanceConstraint::getJacobianMatrix() const
	{
		glm::vec3 p1 = mRigidBodies[0]->getState().position + mAnchorPoints[0];
		glm::vec3 p2 = mRigidBodies[1]->getState().position + mAnchorPoints[1];
		glm::vec3 distance = p2 - p1;

		glm::vec3 r1xd = glm::cross(mAnchorPoints[0], distance);
		glm::vec3 r2xd = glm::cross(mAnchorPoints[1], distance);

		return {
			-distance.x, -distance.y, -distance.z,
			-r1xd.x, -r1xd.y, -r1xd.z,
			distance.x, distance.y, distance.z,
			r2xd.x, r2xd.y, r2xd.z
		};
	}


	void DistanceConstraint::setAnchorPoints(const std::array<glm::vec3, 2>& anchorPoints)
	{
		mAnchorPoints = anchorPoints;
		mUpdated = true;
	}

}
