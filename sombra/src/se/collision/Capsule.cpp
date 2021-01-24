#include <cassert>
#include <algorithm>
#include "se/collision/Capsule.h"

namespace se::collision {

	Capsule::Capsule(float radius, float height) :
		mRadius(0.0f), mHeight(0.0f), mTransformsMatrix(1.0f), mInverseTransformsMatrix(1.0f), mUpdated(true)
	{
		setRadius(radius);
		setHeight(height);
	}


	void Capsule::setRadius(float radius)
	{
		assert(radius >= 0.0f && "The radius of the capsule can't be smaller than zero.");
		mRadius = radius;
		mUpdated = true;
	}


	void Capsule::setHeight(float height)
	{
		assert(height >= 0.0f && "The height of the capsule can't be smaller than zero.");
		mHeight = height;
		mUpdated = true;
	}



	void Capsule::setTransforms(const glm::mat4& transforms)
	{
		mTransformsMatrix = transforms;
		mInverseTransformsMatrix = glm::inverse(transforms);
		mUpdated = true;
	}


	AABB Capsule::getAABB() const
	{
		glm::vec3 a = glm::vec3(0.0f, mHeight / 2.0f, 0.0f);
		glm::vec3 b = -a;

		a = mTransformsMatrix * glm::vec4(a, 1.0f);
		b = mTransformsMatrix * glm::vec4(b, 1.0f);

		glm::vec3 minimum = glm::min(a, b) - glm::vec3(mRadius);
		glm::vec3 maximum = glm::max(a, b) + glm::vec3(mRadius);

		return { minimum, maximum };
	}


	void Capsule::getFurthestPointInDirection(
		const glm::vec3& direction,
		glm::vec3& pointWorld, glm::vec3& pointLocal
	) const
	{
		glm::vec3 a = glm::vec3(0.0f, mHeight / 2.0f, 0.0f);
		glm::vec3 b = -a;

		a = mTransformsMatrix * glm::vec4(a, 1.0f);
		b = mTransformsMatrix * glm::vec4(b, 1.0f);

		glm::vec3 ab = b - a;
		glm::vec3 d = glm::normalize(direction);
		float t = glm::dot(d, ab) / glm::dot(ab, ab);
		t = std::clamp(t, 0.0f, 1.0f);
		glm::vec3 c = a + t * ab;

		pointWorld = c + mRadius * d;
		pointLocal = mInverseTransformsMatrix * glm::vec4(pointWorld, 1.0f);
	}

}
