#include <limits>
#include <cassert>
#include <algorithm>
#include "fe/collision/ConvexPolyhedron.h"

namespace fe { namespace collision {

	ConvexPolyhedron::ConvexPolyhedron(const std::vector<glm::vec3>& vertices) :
		mVertices(vertices), mVerticesWorld(vertices), mTransformsMatrix(1.0f)
	{
		assert(!vertices.empty() && "The ConvexPolyhedron has to have at least one vertex");
	}


	void ConvexPolyhedron::setTransforms(const glm::mat4& transforms)
	{
		mTransformsMatrix = transforms;

		for (size_t i = 0; i < mVertices.size(); ++i) {
			mVerticesWorld[i] = glm::vec3(transforms * glm::vec4(mVertices[i], 1.0f));
		}
	}


	AABB ConvexPolyhedron::getAABB() const
	{
		AABB ret = AABB(
			glm::vec3( std::numeric_limits<float>::max()),
			glm::vec3(-std::numeric_limits<float>::max())
		);

		for (const glm::vec3& vertex : mVerticesWorld) {
			ret.minimum = glm::min(ret.minimum, vertex);
			ret.maximum = glm::max(ret.maximum, vertex);
		}

		return ret;
	}


	void ConvexPolyhedron::getFurthestPointInDirection(
		const glm::vec3& direction,
		glm::vec3& pointWorld, glm::vec3& pointLocal
	) const
	{
		auto itMax = std::max_element(
			mVerticesWorld.begin(),
			mVerticesWorld.end(),
			[&direction](const glm::vec3& p1, const glm::vec3& p2)
			{
				return glm::dot(p1, direction) < glm::dot(p2, direction);
			}
		);

		int iMax = std::distance(mVerticesWorld.begin(), itMax);

		pointWorld = *itMax;
		pointLocal = mVertices[iMax];
	}

}}
