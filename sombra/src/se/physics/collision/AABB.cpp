#include <limits>
#include <algorithm>
#include "se/physics/collision/AABB.h"

namespace se::physics {

	AABB expand(const AABB& aabb1, const AABB& aabb2)
	{
		AABB ret;

		for (int i = 0; i < 3; ++i) {
			ret.minimum[i] = std::min(aabb1.minimum[i], aabb2.minimum[i]);
			ret.maximum[i] = std::max(aabb1.maximum[i], aabb2.maximum[i]);
		}

		return ret;
	}


	bool overlaps(const AABB& aabb1, const AABB& aabb2, float epsilon)
	{
		const AABB *b1 = &aabb1, *b2 = &aabb2;
		if (b2->minimum.x < b1->minimum.x) { std::swap(b1, b2); }
		bool intersecX = (b1->maximum.x + epsilon >= b2->minimum.x) && (b1->minimum.x - epsilon <= b2->maximum.x);

		if (b2->minimum.y < b1->minimum.y) { std::swap(b1, b2); }
		bool intersecY = (b1->maximum.y + epsilon >= b2->minimum.y) && (b1->minimum.y - epsilon <= b2->maximum.y);

		if (b2->minimum.z < b1->minimum.z) { std::swap(b1, b2); }
		bool intersecZ = (b1->maximum.z + epsilon >= b2->minimum.z) && (b1->minimum.z - epsilon <= b2->maximum.z);

		return (intersecX && intersecY && intersecZ);
	}


	bool intersects(const AABB& aabb, const Ray& ray, float epsilon)
	{
		// Branchless slab method, see https://tavianator.com/2011/ray_box.html
		float tXMin = (aabb.minimum.x - ray.origin.x) * ray.invertedDirection.x;
		float tXMax = (aabb.maximum.x - ray.origin.x) * ray.invertedDirection.x;

		float tMin = std::min(tXMin, tXMax);
		float tMax = std::max(tXMin, tXMax);

		float tYMin = (aabb.minimum.y - ray.origin.y) * ray.invertedDirection.y;
		float tYMax = (aabb.maximum.y - ray.origin.y) * ray.invertedDirection.y;

		tMin = std::max(tMin, std::min(tYMin, tYMax));
		tMax = std::min(tMax, std::max(tYMin, tYMax));

		float tZMin = (aabb.minimum.z - ray.origin.z) * ray.invertedDirection.z;
		float tZMax = (aabb.maximum.z - ray.origin.z) * ray.invertedDirection.z;

		tMin = std::max(tMin, std::min(tZMin, tZMax));
		tMax = std::min(tMax, std::max(tZMin, tZMax));

		return tMax + epsilon > tMin;
	}


	bool isInside(const AABB& aabb, const glm::vec3& point, float epsilon)
	{
		return glm::all(glm::greaterThanEqual(point + epsilon, aabb.minimum))
			&& glm::all(glm::lessThanEqual(point + epsilon, aabb.maximum));
	}


	AABB transform(const AABB& aabb, const glm::mat4& transforms)
	{
		AABB ret{ glm::vec3(std::numeric_limits<float>::max()), glm::vec3(-std::numeric_limits<float>::max()) };

		for (float x : { aabb.minimum.x, aabb.maximum.x }) {
			for (float y : { aabb.minimum.y, aabb.maximum.y }) {
				for (float z : { aabb.minimum.z, aabb.maximum.z }) {
					glm::vec3 cubePoint = transforms * glm::vec4(x, y, z, 1.0f);
					ret.minimum = glm::min(ret.minimum, cubePoint);
					ret.maximum = glm::max(ret.maximum, cubePoint);
				}
			}
		}

		return ret;
	}


	float calculateArea(const AABB& aabb)
	{
		glm::vec3 diagonal = aabb.maximum - aabb.minimum;
		return 2.0f * (diagonal.x * diagonal.y + diagonal.y * diagonal.z + diagonal.z * diagonal.x);
	}

}
