#include <set>
#include <limits>
#include <cassert>
#include <algorithm>
#include <glm/gtc/constants.hpp>
#include "fe/collision/MeshCollider.h"
#include "Triangle.h"

namespace fe { namespace collision {

	MeshCollider::MeshCollider(
		const std::vector<glm::vec3>& vertices,
		const std::vector<unsigned short>& indices,
		ConvexStrategy strategy
	) : mTransformsMatrix(1.0f)
	{
		assert(indices.size() % 3 == 0 && "The faces of the MeshCollider must be triangles");

		//calculateAABBs();
		if (strategy == ConvexStrategy::QuickHull) {
			doQuickHull(vertices);
		}

		calculateAABB();
	}


	void MeshCollider::setTransforms(const glm::mat4& transforms)
	{
		mTransformsMatrix = transforms;

		for (ConvexPolyhedron& convexPart : mConvexParts) {
			convexPart.setTransforms(transforms);
		}

		calculateAABB();
	}


	std::vector<const ConvexCollider*> MeshCollider::getOverlapingParts(const AABB& aabb) const
	{
		std::vector<const ConvexCollider*> overlapingParts;

		for (const ConvexPolyhedron& convexPart : mConvexParts) {
			if (overlaps(aabb, convexPart.getAABB())) {
				overlapingParts.push_back(&convexPart);
			}
		}

		return overlapingParts;
	}

// Private functions
	void MeshCollider::calculateAABB()
	{
		mAABB = {
			glm::vec3( std::numeric_limits<float>::max()),
			glm::vec3(-std::numeric_limits<float>::max())
		};

		for (const ConvexPolyhedron& convexPart : mConvexParts) {
			AABB convexPartAABB = convexPart.getAABB();
			mAABB.minimum = glm::min(mAABB.minimum, convexPartAABB.minimum);
			mAABB.maximum = glm::max(mAABB.maximum, convexPartAABB.maximum);
		}
	}


	std::vector<glm::vec3> MeshCollider::doQuickHull(
		const std::vector<glm::vec3>& points
	) const
	{
		std::vector<glm::vec3> convexHull = createInitialHull(points);


		return convexHull;
	}


	std::vector<glm::vec3> MeshCollider::createInitialHull(
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


	MeshCollider::EdgeMatrix MeshCollider::createDualGraph(const std::vector<int>& indices) const
	{
		const int nTriangles = indices.size() / 3;

		EdgeMatrix graphEdges(nTriangles, std::vector<bool>(nTriangles, false));
		for (int t1 = 0; t1 < nTriangles; ++t1) {
			for (int t2 = t1 + 1; t2 < nTriangles; ++t2) {
				for (int i = 0; i < 3; ++i) {
					int t1p1 = indices[3*t1 + i], t1p2 = indices[3*t1 + (i+1) % 3],
						t2p1 = indices[3*t2 + i], t2p2 = indices[3*t2 + (i+1) % 3];

					if (t1p1 == t2p2) { std::swap(t2p1, t2p2); }
					if ((t1p1 == t2p1) && (t1p2 == t2p2)) {
						graphEdges[t1][t2] = true;
					}
				}
			}
		}

		return graphEdges;
	}


	void MeshCollider::halfEdgeCollapse(int v1, int v2, EdgeMatrix& graphEdges) const
	{
		int nVertices = graphEdges.size();

		assert((v1 >= 0 && v1 < nVertices) && (v2 >= 0 && v2 < nVertices)
			&& "The indexes of the vertices can't be out of range");

		for (int iRow = 0; iRow < nVertices; ++iRow) {
			if (graphEdges[iRow][v2]) {
				graphEdges[iRow][v2] = false;
				if (iRow != v1) {
					graphEdges[iRow][v1] = true;
				}
			}
		}
	}


	float MeshCollider::calculateCost(
		int v1, int v2,
		const std::vector<glm::vec3>& vertices,
		const std::vector<unsigned short>& indices
	) const
	{
		float normalizationFactor = calculateNormalizationFactor();
		float alphaFactor = calculateAspectRatioFactor(normalizationFactor);

		return calculateConcavity(v1, v2) / normalizationFactor
			+ alphaFactor * calculateAspectRatio(v1, v2, vertices, indices);
	}


	float MeshCollider::calculateConcavity(int /*v1*/, int /*v2*/) const
	{
		return 0.0f;
	}


	float MeshCollider::calculateAspectRatio(
		int t1, int t2,
		const std::vector<glm::vec3>& vertices,
		const std::vector<unsigned short>& indices
	) const
	{
		int nTriangles = indices.size() / 3;
		assert((t1 >= 0 && t1 < nTriangles) && (t2 >= 0 && t2 < nTriangles)
			&& "The indexes of the triangles can't be out of range");

		typedef std::set<int> Edge;
		std::array<Edge, 3> edges1;
		for (int i = 0; i < 3; ++i) {
			edges1[i] = { indices[3*t1 + i], indices[3*t1 + (i+1)%3] };
		}
		std::array<Edge, 3> edges2;
		for (int i = 0; i < 3; ++i) {
			edges2[i] = { indices[3*t2 + i], indices[3*t2 + (i+1)%3] };
		}

		// 1. Calculate the perimeter of the surface (excluding the internal
		// edges of the triangles of the graph vertices v and w)
		std::vector<Edge> edges;
		std::set_symmetric_difference(edges1.begin(), edges1.end(), edges2.begin(), edges2.end(), edges.begin());

		float perimeter = 0.0f;
		for (const Edge& e : edges) {
			perimeter += glm::length(vertices[*(++e.begin())] - vertices[*e.begin()]);
		}

		// 2. Calculate the area of the surface as the sum of the areas of the
		// triangles of the graph vertices v and w
		float area1 = calculateTriangleArea({
			vertices[*edges1[0].begin()],
			vertices[*edges1[1].begin()],
			vertices[*edges1[2].begin()]
		});

		float area2 = calculateTriangleArea({
			vertices[*edges2[0].begin()],
			vertices[*edges2[1].begin()],
			vertices[*edges2[2].begin()]
		});

		float area = area1 + area2;

		return std::pow(perimeter, 2) / (4 * glm::pi<float>() * area);
	}


	float MeshCollider::calculateNormalizationFactor() const
	{
		return glm::length(mAABB.maximum - mAABB.minimum);
	}


	float MeshCollider::calculateAspectRatioFactor(float normalizationFactor) const
	{
		return sMinimumConcavity / (10.0f * normalizationFactor);
	}

}}
