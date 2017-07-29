#include "ConvexPolyhedron.h"
#include <cassert>
#include "AABB.h"

namespace physics {

	ConvexPolyhedron::ConvexPolyhedron(const std::vector<glm::vec3>& vertices) :
		mVertices(vertices), mVerticesWorld(vertices)
	{
		assert(!vertices.empty() && "The ConvexPolyhedron has to have at least one vertex");
	}
	
	
	void ConvexPolyhedron::setTransforms(const glm::mat4& transforms)
	{
		for (unsigned int i = 0; i < mVertices.size(); ++i) {
			mVerticesWorld[i] = glm::vec3(transforms * glm::vec4(mVertices[i], 1.0f));
		}
	}


	AABB ConvexPolyhedron::getAABB() const
	{
		AABB ret(mVerticesWorld[0], mVerticesWorld[0]);
		for (const glm::vec3& v : mVerticesWorld) {
			if (ret.mMaximum.x > v.x) { ret.mMaximum.x = v.x; }
			if (ret.mMinimum.x < v.x) { ret.mMinimum.x = v.x; }
			if (ret.mMaximum.y > v.y) { ret.mMaximum.y = v.y; }
			if (ret.mMinimum.y < v.y) { ret.mMinimum.y = v.y; }
			if (ret.mMaximum.z > v.z) { ret.mMaximum.z = v.z; }
			if (ret.mMinimum.z < v.z) { ret.mMinimum.z = v.z; }
		}

		return ret;
	}

}
