#include "AxisAlignedBoundingBox.h"

namespace physics {

	void AxisAlignedBoundingBox::setTransforms(const glm::mat4& transforms)
	{
		mVertices[0] = glm::vec3(-mLengths.x, -mLengths.y, -mLengths.z) / 2.0f;
		mVertices[1] = glm::vec3(-mLengths.x, -mLengths.y,  mLengths.z) / 2.0f;
		mVertices[2] = glm::vec3(-mLengths.x,  mLengths.y, -mLengths.z) / 2.0f;
		mVertices[3] = glm::vec3(-mLengths.x,  mLengths.y,  mLengths.z) / 2.0f;
		mVertices[4] = glm::vec3( mLengths.x, -mLengths.y, -mLengths.z) / 2.0f;
		mVertices[5] = glm::vec3( mLengths.x, -mLengths.y,  mLengths.z) / 2.0f;
		mVertices[6] = glm::vec3( mLengths.x,  mLengths.y, -mLengths.z) / 2.0f;
		mVertices[7] = glm::vec3( mLengths.x,  mLengths.y,  mLengths.z) / 2.0f;

		for (unsigned int i = 0; i < 8; ++i) {
			mVertices[i] = glm::vec3(transforms * glm::vec4(mVertices[i], 0));
		}
	}

}
