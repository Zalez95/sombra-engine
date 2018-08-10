#include <limits>
#include "fe/collision/ConvexPolyhedron.h"

namespace fe { namespace collision {

	void ConvexPolyhedron::setTransforms(const glm::mat4& transforms)
	{
		mTransformsMatrix = transforms;

		mMesh.vertices = mLocalVertices;
		for (HEVertex& vertex : mMesh.vertices) {
			vertex.location = mTransformsMatrix * glm::vec4(vertex.location, 1.0);
		}
	}


	AABB ConvexPolyhedron::getAABB() const
	{
		AABB ret{
			glm::vec3( std::numeric_limits<float>::max()),
			glm::vec3(-std::numeric_limits<float>::max())
		};

		for (const HEVertex& vertex : mMesh.vertices) {
			ret.minimum = glm::min(ret.minimum, vertex.location);
			ret.maximum = glm::max(ret.maximum, vertex.location);
		}

		return ret;
	}


	void ConvexPolyhedron::getFurthestPointInDirection(
		const glm::vec3& direction,
		glm::vec3& pointWorld, glm::vec3& pointLocal
	) const
	{
		int iVertex = getFurthestVertexInDirection(mMesh, direction);

		pointWorld = mMesh.vertices[iVertex].location;
		pointLocal = mLocalVertices[iVertex].location;
	}

}}
