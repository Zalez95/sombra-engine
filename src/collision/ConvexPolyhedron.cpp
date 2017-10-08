#include "ConvexPolyhedron.h"
#include <cassert>
#include <algorithm>
#include "AABB.h"

namespace collision {

	ConvexPolyhedron::ConvexPolyhedron(const std::vector<glm::vec3>& vertices) :
		mVertices(vertices), mVerticesWorld(vertices)
	{
		assert(!vertices.empty() && "The ConvexPolyhedron has to have at least one vertex");
	}
	
	
	void ConvexPolyhedron::setTransforms(const glm::mat4& transforms)
	{
		for (size_t i = 0; i < mVertices.size(); ++i) {
			mVerticesWorld[i] = glm::vec3(transforms * glm::vec4(mVertices[i], 1.0f));
		}
	}


	AABB ConvexPolyhedron::getAABB() const
	{
		AABB ret(mVerticesWorld[0], mVerticesWorld[0]);
		for (const glm::vec3 p : mVerticesWorld) {
			if (p.x > ret.mMaximum.x) { ret.mMaximum.x = p.x; }
			if (p.x < ret.mMinimum.x) { ret.mMinimum.x = p.x; }
			if (p.y > ret.mMaximum.y) { ret.mMaximum.y = p.y; }
			if (p.y < ret.mMinimum.y) { ret.mMinimum.y = p.y; }
			if (p.z > ret.mMaximum.z) { ret.mMaximum.z = p.z; }
			if (p.z < ret.mMinimum.z) { ret.mMinimum.z = p.z; }
		}

		return ret;
	}


	glm::vec3 ConvexPolyhedron::getFurthestPointInDirection(
		const glm::vec3& direction
	) const
	{
		return *std::max_element(
			mVerticesWorld.begin(),
			mVerticesWorld.end(),
			[direction](const glm::vec3& p1, const glm::vec3& p2)
			{
				return glm::dot(p1, direction) < glm::dot(p2, direction);
			}
		);
	}

}
