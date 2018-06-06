#include <set>
#include <algorithm>
#include <glm/gtc/constants.hpp>
#include "HACD.h"
#include "Triangle.h"

namespace fe { namespace collision {

	HACD::EdgeMatrix HACD::createDualGraph(const std::vector<int>& indices) const
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


	void HACD::halfEdgeCollapse(int v1, int v2, EdgeMatrix& graphEdges) const
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


	float HACD::calculateCost(
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


	float HACD::calculateConcavity(int /*v1*/, int /*v2*/) const
	{
		return 0.0f;
	}


	float HACD::calculateAspectRatio(
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


	float HACD::calculateNormalizationFactor() const
	{
		return 0.0f;//glm::length(mAABB.maximum - mAABB.minimum);
	}


	float HACD::calculateAspectRatioFactor(float normalizationFactor) const
	{
		return sMinimumConcavity / (10.0f * normalizationFactor);
	}

}}
