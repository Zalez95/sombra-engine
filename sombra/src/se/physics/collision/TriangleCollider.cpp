#include <limits>
#include "se/physics/collision/TriangleCollider.h"

namespace se::physics {

	void TriangleCollider::setLocalVertices(const std::array<glm::vec3, 3>& vertices)
	{
		mLocalVertices = vertices;
		setTransforms(mTransformsMatrix);
	}


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
		return {
			glm::min(glm::min(mWorldVertices[0], mWorldVertices[1]), mWorldVertices[2]),
			glm::max(glm::max(mWorldVertices[0], mWorldVertices[1]), mWorldVertices[2])
		};
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
				iMaxVertex = static_cast<int>(i);
			}
		}

		pointWorld = mWorldVertices[iMaxVertex];
		pointLocal = mLocalVertices[iMaxVertex];
	}

}
