#include <array>
#include <algorithm>
#include "QuickHull.h"

namespace fe { namespace collision {

	QuickHull::QuickHull(
		const std::vector<glm::vec3>& vertices,
		const std::vector<unsigned short>& /*indices*/
	) {
		/*auto convexHull =*/ createInitialHull(vertices);
	}


	std::vector<glm::vec3> QuickHull::createInitialHull(
		const std::vector<glm::vec3>& points
	) const
	{
		size_t iP1, iP2, iP3, iP4;
		
		// 1. Find the extreme points in each axis
		std::array<size_t, 6> indexExtremePoints{};
		for (size_t i = 0; i < points.size(); ++i) {
			for (size_t j = 0; j < 3; ++j) {
				if (points[i][j] < points[indexExtremePoints[2*j]][j]) {
					indexExtremePoints[2*j] = i;
				}
				if (points[i][j] > points[indexExtremePoints[2*j + 1]][j]) {
					indexExtremePoints[2*j + 1] = i;
				}
			}
		}

		// 2. Find from the extreme points the pair which are furthest apart
		float maxLength = -std::numeric_limits<float>::max();
		for (size_t i = 0; i < points.size(); ++i) {
			for (size_t j = i + 1; j < points.size(); ++j) {
				float currentLength = glm::length(points[j] - points[i]);
				if (currentLength > maxLength) {
					iP1 = i; iP2 = j;
					maxLength = currentLength;
				}
			}
		}

		// 3. Find the furthest point to the edge between the last 2 points
		glm::vec3 dirP1P2 = glm::normalize(points[iP2] - points[iP1]);
		maxLength = -std::numeric_limits<float>::max();
		for (size_t i = 0; i < points.size(); ++i) {
			glm::vec3 projection = points[iP1] + dirP1P2 * glm::dot(points[i], dirP1P2);
			float currentLength = glm::length(points[i] - projection);
			if (currentLength > maxLength) {
				iP3 = i;
				maxLength = currentLength;
			}
		}

		// 4. Find the furthest point to the triangle created from the last 3
		// points
		glm::vec3 dirP1P3 = glm::normalize(points[iP3] - points[iP1]);
		glm::vec3 tNormal = glm::normalize(glm::cross(dirP1P2, dirP1P3));
		maxLength = -std::numeric_limits<float>::max();
		for (size_t i = 0; i < points.size(); ++i) {
			float currentLength = std::abs(glm::dot(points[i], tNormal));
			if (currentLength > maxLength) {
				iP4 = i;
				maxLength = currentLength;
			}
		}

		return { points[iP1], points[iP2], points[iP3], points[iP4] };
	}


	void QuickHull::addFaceToHull(
		const Face& /*face*/,
		const std::vector<glm::vec3>& /*vertices*/
	) {
		//glm::vec3 v01 = vertices[face[1]] - vertices[face[0]];
		//glm::vec3 v02 = vertices[face[2]] - vertices[face[0]];
		//glm::vec3 faceNormal = glm::normalize(glm::cross(v01, v02));
		//std::max_element(vertices.begin(), vertices.end(), )
	}

}}
