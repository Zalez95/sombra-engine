#include <limits>
#include "se/collision/TriangleCollider.h"

namespace se::collision {

	void TriangleCollider::setTransforms(const glm::mat4& transforms)
	{
		mTransformsMatrix = transforms;

		for (std::size_t i = 0; i < mLocalVertices.size(); ++i) {
			mWorldVertices[i] = mTransformsMatrix * glm::vec4(mLocalVertices[i], 1.0f);
		}

		mUpdated = true;
	}


	AABB TriangleCollider::getAABB() const
	{
		AABB triangleAABB = {
			glm::vec3( std::numeric_limits<float>::max()),
			glm::vec3(-std::numeric_limits<float>::max())
		};

		for (const glm::vec3& vertex : mWorldVertices) {
			triangleAABB.minimum = glm::min(triangleAABB.minimum, vertex);
			triangleAABB.maximum = glm::max(triangleAABB.maximum, vertex);
		}

		return triangleAABB;
	}


	void TriangleCollider::getFurthestPointInDirection(
		const glm::vec3& direction,
		glm::vec3& pointWorld, glm::vec3& pointLocal
	) const
	{
		int iMaxVertex = -1;
		float maxDot = -std::numeric_limits<float>::max();
		for (std::size_t i = 0; i < mWorldVertices.size(); ++i) {
			float currentDot = glm::dot(direction, mWorldVertices[i]);
			if (currentDot > maxDot) {
				maxDot = currentDot;
				iMaxVertex = i;
			}
		}

		pointWorld = mWorldVertices[iMaxVertex];
		pointLocal = mLocalVertices[iMaxVertex];
	}

}
