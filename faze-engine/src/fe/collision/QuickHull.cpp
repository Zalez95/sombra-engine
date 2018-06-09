#include <list>
#include <limits>
#include <algorithm>
#include "QuickHull.h"

namespace fe { namespace collision {

	glm::vec3 calculateTriangleNormal(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
	{
		return glm::normalize(glm::cross(b - a, c - a));
	}


	QuickHull::QuickHull(
		const std::vector<glm::vec3>& vertices,
		const std::vector<unsigned short>& /*indices*/
	) {
		for (const glm::vec3& v : createInitialHull(vertices)) {
			mMesh.addVertex(v);
		}
		mMesh.addFace({ 0, 1, 2 });
		mMesh.addFace({ 0, 3, 1 });
		mMesh.addFace({ 0, 2, 3 });
		mMesh.addFace({ 1, 3, 2 });

		std::list<std::vector<glm::vec3>> outsideList;

		for (auto& currentFace : mMesh.faces) {
			// 1. Get the vertices outside of the current Hull by the given face
			auto verticesOutside = getVerticesOutside(currentFace, vertices);
			if (!verticesOutside.empty()) {
				// 2. Get the furthest point in the direction of the face normal
				const glm::vec3 eyePoint = getFurthestPoint(currentFace, verticesOutside);

				// 3. Calculate the horizon from the current point
				std::vector<Edge> horizon;
				std::vector<glm::vec3> unclaimedVertices;
				calculateHorizon(eyePoint, currentFace, horizon);

				// 4. Create the new faces of the hull by joining the edges of
				// the horizon with the eyePoint
				for (const Edge& horizonEdge : horizon) {
					//mMesh.faces.emplace_back(eyePoint, horizonEdge );
					//verticesOutside = getVerticesOutside(newFace, unclaimedVertices);
				}
			}
		}
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


	std::vector<glm::vec3> QuickHull::getVerticesOutside(
		const Face& face,
		const std::vector<glm::vec3>& vertices
	) const
	{
		std::vector<glm::vec3> verticesOutside;

		for (const glm::vec3& currentVertex : vertices) {
			float currentDistance = glm::dot(currentVertex - mVertices[face.ab.p1], face.normal);
			if (currentDistance == 0) {
				if (
					(mVertices[face.ab.p1] != currentVertex)
					&& (mVertices[face.bc.p1] != currentVertex)
					&& (mVertices[face.ca.p1] != currentVertex)
				) {
					verticesOutside.push_back(currentVertex);
				}
			}
			else if (currentDistance > 0) {
				verticesOutside.push_back(currentVertex);
			}
		}

		return verticesOutside;
	}


	glm::vec3 QuickHull::getFurthestPoint(
		const Face& face,
		const std::vector<glm::vec3>& vertices
	) const
	{
		glm::vec3 ret;

		float maxDistance = -std::numeric_limits<float>::max();
		for (const glm::vec3& currentVertex : vertices) {
			float currentDistance = glm::dot(currentVertex - mVertices[face.ab.p1], face.normal);
			if (currentDistance > currentDistance) {
				maxDistance = currentDistance;
				ret = currentVertex;
			}
		}

		return ret;
	}


	void QuickHull::calculateHorizon(
		const glm::vec3& /*eyePoint*/, const Face& /*face*/,
		const std::vector<Edge>& /*horizon*/
	) const
	{
		/*if (face not in ConvexHull) {
			set crossedEdge not in convex hull
			return;
		}

		if (glm::dot(face.normal, eyePoint) >= 0) {
			set curFace not in convex hull
			uncalimedVertices.add(outsideVertices);
			if (crossedEdge) {
				set crossedEdge not in convex hull
			}
			
		}*/
	}

}}
