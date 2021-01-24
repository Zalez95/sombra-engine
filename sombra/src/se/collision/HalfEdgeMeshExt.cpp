#include <array>
#include <stack>
#include <limits>
#include <numeric>
#include <algorithm>
#include <glm/gtc/constants.hpp>
#include "se/collision/AABB.h"
#include "se/collision/HalfEdgeMeshExt.h"

#define NORMALIZATION_ZERO 0.0001f

namespace se::collision {

	std::pair<bool, std::string> validateMesh(const HalfEdgeMesh& meshData)
	{
		bool isMeshValid = true;
		std::string failReason;

		// Check if the HEEdges are valid
		auto itVertexEdgePair = meshData.vertexEdgeMap.begin();
		for (auto itEdge = meshData.edges.begin(); (itEdge != meshData.edges.end()) && isMeshValid; ++itEdge) {
			if (!meshData.vertices.isActive(itEdge->vertex)
				|| !meshData.edges.isActive(itEdge->oppositeEdge)
				|| (itVertexEdgePair = std::find_if(
						meshData.vertexEdgeMap.begin(), meshData.vertexEdgeMap.end(),
						[&](const std::pair<std::pair<int, int>, int>& pair) {
							return pair.second == static_cast<int>(itEdge.getIndex());
						}
					)) == meshData.vertexEdgeMap.end()
				|| (itVertexEdgePair->first.second != itEdge->vertex)
				|| (itVertexEdgePair = meshData.vertexEdgeMap.find(
						{ itVertexEdgePair->first.second, itVertexEdgePair->first.first }
					)) == meshData.vertexEdgeMap.end()
				|| (itVertexEdgePair->second != itEdge->oppositeEdge)
			) {
				isMeshValid = false;
				failReason = "Found invalid HEEdge " + std::to_string(itEdge.getIndex());
			}
		}

		// Check if the HEFaces are valid
		for (auto itFace = meshData.faces.begin(); (itFace != meshData.faces.end()) && isMeshValid; ++itFace) {
			std::vector<int> visitedVertices;
			int iInitialEdge = itFace->edge;
			int iCurrentEdge = iInitialEdge;
			do {
				if (!meshData.edges.isActive(iCurrentEdge)) {
					isMeshValid = false;
					failReason = "Found HEFace " + std::to_string(itFace.getIndex()) + " without a valid initial HEEdge";
				}
				else {
					const HEEdge& currentEdge = meshData.edges[iCurrentEdge];

					if (!meshData.edges.isActive(currentEdge.previousEdge)
						|| !meshData.edges.isActive(currentEdge.nextEdge)
						|| (meshData.edges[currentEdge.previousEdge].vertex != meshData.edges[currentEdge.oppositeEdge].vertex)
						|| std::any_of(
								visitedVertices.begin(), visitedVertices.end(),
								[&](int iVertex) { return iVertex == currentEdge.vertex; }
							)
					) {
						isMeshValid = false;
						failReason = "Found HEFace " + std::to_string(itFace.getIndex())
							+ " with an invalid HEEdge loop at HEEdge " + std::to_string(iCurrentEdge);
					}
					visitedVertices.push_back(currentEdge.vertex);

					iCurrentEdge = currentEdge.nextEdge;
				}
			}
			while ((iCurrentEdge != iInitialEdge) && isMeshValid);
		}

		return std::make_pair(isMeshValid, failReason);
	}


	HalfEdgeMesh triangulateFaces(const HalfEdgeMesh& originalMesh)
	{
		HalfEdgeMesh triangulatedMesh;
		triangulatedMesh.vertices = originalMesh.vertices;

		for (const HEFace& face : originalMesh.faces) {
			const HEEdge& initialEdge = originalMesh.edges[face.edge];
			int iLastEdge		= initialEdge.previousEdge;
			int iCurrentEdge	= initialEdge.nextEdge;
			int iInitialVertex	= originalMesh.edges[initialEdge.oppositeEdge].vertex;
			do {
				const HEEdge& currentEdge	= originalMesh.edges[iCurrentEdge];
				const HEEdge& oppositeEdge	= originalMesh.edges[currentEdge.oppositeEdge];

				std::array<int, 3> vertexIndices = { iInitialVertex, oppositeEdge.vertex, currentEdge.vertex };
				addFace(triangulatedMesh, vertexIndices.begin(), vertexIndices.end());

				iCurrentEdge = currentEdge.nextEdge;
			}
			while (iCurrentEdge != iLastEdge);
		}

		return triangulatedMesh;
	}


	glm::vec3 calculateVertexNormal(
		const HalfEdgeMesh& meshData, const utils::PackedVector<glm::vec3>& faceNormals,
		int iVertex
	) {
		glm::vec3 normal(0.0f);

		if (meshData.vertices.isActive(iVertex)) {
			int iInitialEdge = meshData.vertices[iVertex].edge;
			int iCurrentEdge = iInitialEdge;

			if (meshData.edges.isActive(iInitialEdge)) {
				do {
					const HEEdge& currentEdge = meshData.edges[iCurrentEdge];

					if (meshData.faces.isActive(currentEdge.face)) {
						normal += faceNormals[currentEdge.face];
					}

					iCurrentEdge = (currentEdge.oppositeEdge < 0)? -1 : meshData.edges[currentEdge.oppositeEdge].nextEdge;
				}
				while ((iCurrentEdge != iInitialEdge) && meshData.edges.isActive(iCurrentEdge));

				// If the loop wasn't completed check in the other direction
				if ((iCurrentEdge < 0)
					&& (meshData.edges.isActive(iCurrentEdge = meshData.edges[iInitialEdge].previousEdge))
					&& (meshData.edges.isActive(iCurrentEdge = meshData.edges[iCurrentEdge].oppositeEdge))
				) {
					do {
						const HEEdge& currentEdge = meshData.edges[iCurrentEdge];

						if (currentEdge.face >= 0) {
							normal += faceNormals[currentEdge.face];
						}

						iCurrentEdge = (currentEdge.previousEdge < 0)? -1 : meshData.edges[currentEdge.previousEdge].oppositeEdge;
					}
					while ((iCurrentEdge != iInitialEdge) && meshData.edges.isActive(iCurrentEdge));
				}
			}
		}

		float normalLength = glm::length(normal);
		return (normalLength < NORMALIZATION_ZERO)? normal : normal / normalLength;
	}


	glm::vec3 calculateFaceNormal(const HalfEdgeMesh& meshData, int iFace)
	{
		glm::vec3 normal(0.0f);

		if (meshData.faces.isActive(iFace)) {
			int iInitialEdge = meshData.faces[iFace].edge;
			int iCurrentEdge = iInitialEdge;
			do {
				const HEEdge& currentEdge = meshData.edges[iCurrentEdge];
				const HEEdge& oppositeEdge = meshData.edges[currentEdge.oppositeEdge];
				glm::vec3 p1 = meshData.vertices[oppositeEdge.vertex].location;
				glm::vec3 p2 = meshData.vertices[currentEdge.vertex].location;

				normal.x += (p1.y - p2.y) * (p1.z + p2.z);
				normal.y += (p1.z - p2.z) * (p1.x + p2.x);
				normal.z += (p1.x - p2.x) * (p1.y + p2.y);

				iCurrentEdge = currentEdge.nextEdge;
			}
			while (iCurrentEdge != iInitialEdge);
		}

		float normalLength = glm::length(normal);
		return (normalLength < NORMALIZATION_ZERO)? normal : normal / normalLength;
	}


	glm::vec3 calculateFaceCentroid(const HalfEdgeMesh& meshData, int iFace)
	{
		glm::vec3 centroid(0.0f);

		if (meshData.faces.isActive(iFace)) {
			float vertexCount = 0.0f;

			int iInitialEdge = meshData.faces[iFace].edge;
			int iCurrentEdge = iInitialEdge;
			do {
				const HEEdge& currentEdge = meshData.edges[iCurrentEdge];

				centroid += meshData.vertices[currentEdge.vertex].location;
				vertexCount++;

				iCurrentEdge = currentEdge.nextEdge;
			}
			while (iCurrentEdge != iInitialEdge);

			centroid /= vertexCount;
		}

		return centroid;
	}


	float calculateFaceArea(const HalfEdgeMesh& meshData, int iFace)
	{
		float area = 0.0f;

		if (meshData.faces.isActive(iFace)) {
			const HEEdge& initialEdge = meshData.edges[ meshData.faces[iFace].edge ];
			int iLastEdge		= initialEdge.previousEdge;
			int iCurrentEdge	= initialEdge.nextEdge;
			int iInitialVertex	= meshData.edges[initialEdge.oppositeEdge].vertex;
			do {
				const HEEdge& currentEdge	= meshData.edges[iCurrentEdge];
				const HEEdge& oppositeEdge	= meshData.edges[currentEdge.oppositeEdge];

				const glm::vec3& p1 = meshData.vertices[iInitialVertex].location;
				const glm::vec3& p2 = meshData.vertices[oppositeEdge.vertex].location;
				const glm::vec3& p3 = meshData.vertices[currentEdge.vertex].location;

				area += utils::calculateTriangleArea({ p1, p2, p3 });

				iCurrentEdge = currentEdge.nextEdge;
			}
			while (iCurrentEdge != iLastEdge);
		}

		return area;
	}


	AABB calculateAABB(const HalfEdgeMesh& meshData)
	{
		AABB meshAABB = {
			glm::vec3( std::numeric_limits<float>::max()),
			glm::vec3(-std::numeric_limits<float>::max())
		};

		for (const HEVertex& vertex : meshData.vertices) {
			meshAABB.minimum = glm::min(meshAABB.minimum, vertex.location);
			meshAABB.maximum = glm::max(meshAABB.maximum, vertex.location);
		}

		return meshAABB;
	}


	glm::vec3 calculateCentroid(const HalfEdgeMesh& meshData)
	{
		glm::vec3 centroid = std::accumulate(
			meshData.vertices.begin(), meshData.vertices.end(), glm::vec3(0.0f),
			[](const HEVertex& v1, const HEVertex& v2) { return v1.location + v2.location; }
		);

		if (!meshData.vertices.empty()) {
			centroid /= static_cast<float>(meshData.vertices.size());
		}

		return centroid;
	}


	float calculateArea(const HalfEdgeMesh& meshData)
	{
		float area = 0.0f;
		for (auto itFace = meshData.faces.begin(); itFace != meshData.faces.end(); ++itFace) {
			area += calculateFaceArea(meshData, itFace.getIndex());
		}
		return area;
	}


	float calculateVolume(
		const HalfEdgeMesh& meshData,
		const utils::PackedVector<glm::vec3>& faceNormals
	) {
		// Get the centroid of the mesh
		glm::vec3 centroid = calculateCentroid(meshData);

		// Calculate the sum of the volumes of the pyramids created with the
		// HEFaces of the mesh and the centroid
		float volume = 0.0f;
		for (auto itFace = meshData.faces.begin(); itFace != meshData.faces.end(); ++itFace) {
			// Calculate the pyramid height
			const glm::vec3& facePoint = meshData.vertices[ meshData.edges[itFace->edge].vertex ].location;
			const glm::vec3& faceNormal = faceNormals[itFace.getIndex()];
			glm::vec3 centroidToFacePoint = facePoint - centroid;
			float distance = glm::length(centroidToFacePoint);
			if (distance > 0.0f) {
				centroidToFacePoint /= distance;
			}

			float dotCPN = glm::dot(centroidToFacePoint, faceNormal);
			float height = (dotCPN > 0.0f)? dotCPN * distance : 0.0f;

			// Calculate the pyramid base area
			float baseArea = calculateFaceArea(meshData, itFace.getIndex());

			volume += glm::third<float>() * baseArea * height;
		}

		return volume;
	}


	int getFurthestVertexInDirection(
		const HalfEdgeMesh& meshData,
		const glm::vec3& direction
	) {
		auto getVertexDistance = [&direction](const glm::vec3& location) {
			return glm::dot(location, direction);
		};

		int iBestVertex = meshData.vertices.begin().getIndex();
		float bestDistance = getVertexDistance(meshData.vertices[iBestVertex].location);

		for (bool end = false; !end;) {
			// Search the best neighbour of the current vertex
			int iInitialEdge = meshData.vertices[iBestVertex].edge, iCurrentEdge = iInitialEdge,
				iBestVertex2 = -1;
			float bestDistance2 = -std::numeric_limits<float>::max();
			do {
				HEEdge currentEdge = meshData.edges[iCurrentEdge];
				HEVertex currentVertex = meshData.vertices[currentEdge.vertex];

				float currentDistance = getVertexDistance(currentVertex.location);
				if (currentDistance > bestDistance2) {
					bestDistance2 = currentDistance;
					iBestVertex2 = currentEdge.vertex;
				}

				iCurrentEdge = meshData.edges[currentEdge.oppositeEdge].nextEdge;
			}
			while ((iCurrentEdge != iInitialEdge) && meshData.edges.isActive(iCurrentEdge));

			if (bestDistance2 > bestDistance) {
				// Update our upper bound
				bestDistance = bestDistance2;
				iBestVertex = iBestVertex2;
			}
			else {
				// Found maximum
				end = true;
			}
		}

		return iBestVertex;
	}


	std::pair<std::vector<int>, std::vector<int>> calculateHorizon(
		const HalfEdgeMesh& meshData, const utils::PackedVector<glm::vec3>& faceNormals,
		const glm::vec3& eyePoint, int iInitialFace
	) {
		std::vector<int> horizonEdges, visibleFaces;

		// Test the visibility of the initial HEFace
		const HEFace& initialFace = meshData.faces[iInitialFace];
		const HEVertex& initialFaceVertex = meshData.vertices[ meshData.edges[initialFace.edge].vertex ];
		const glm::vec3& initialFaceNormal = faceNormals[iInitialFace];
		if (glm::dot(eyePoint - initialFaceVertex.location, initialFaceNormal) > 0.0f) {
			visibleFaces.push_back(iInitialFace);

			// Search the visible HEFaces and horizon HEEdges using the DFS
			// algorithm
			std::stack<int> edgesToEvaluate;
			edgesToEvaluate.push(initialFace.edge);
			std::vector<int> visitedFaces = { iInitialFace };
			do {
				int iCurrentEdge = edgesToEvaluate.top();
				edgesToEvaluate.pop();

				const HEEdge& currentEdge = meshData.edges[iCurrentEdge];
				const HEEdge& oppositeEdge = meshData.edges[currentEdge.oppositeEdge];

				// 1. Check if the current HEFace hasn't been visited
				if (std::find(visitedFaces.begin(), visitedFaces.end(), oppositeEdge.face) == visitedFaces.end()) {
					// 1.1. Mark the current HEFace as visited
					visitedFaces.push_back(oppositeEdge.face);

					// 1.2. Test the visibility of the current HEFace from the
					// eye point
					const HEVertex& oppositeFaceVertex = meshData.vertices[oppositeEdge.vertex];
					const glm::vec3& oppositeFaceNormal = faceNormals[oppositeEdge.face];
					if (glm::dot(eyePoint - oppositeFaceVertex.location, oppositeFaceNormal) > 0.0f) {
						// 1.2.1. Mark the HEFace as visible and continue
						// searching in the opposite HEFace
						visibleFaces.push_back(oppositeEdge.face);
						edgesToEvaluate.push(iCurrentEdge);
						edgesToEvaluate.push(oppositeEdge.nextEdge);
					}
					else {
						// 1.2.2. Mark the HEEdge as an horizon one and continue
						// searching in the current HEFace
						horizonEdges.push_back(iCurrentEdge);
						edgesToEvaluate.push(currentEdge.nextEdge);
					}
				}
				else {
					// 1.3. Check if we are in the returning stage of the
					// horizon algorithm
					if (!horizonEdges.empty()) {
						// Check if the opposite HEEdge is the same than the
						// parent one
						if (!edgesToEvaluate.empty()
							&& (oppositeEdge.face == meshData.edges[edgesToEvaluate.top()].face)
						) {
							// 1.3.1. Continue searching in the parent HEFace
							edgesToEvaluate.pop();
							edgesToEvaluate.push(oppositeEdge.nextEdge);
						}
						else  {
							// 1.3.2. Continue searching in the current HEFace
							if (std::find(visibleFaces.begin(), visibleFaces.end(), oppositeEdge.face) == visibleFaces.end()) {
								// Mark the current HEEdge as an horizon one
								horizonEdges.push_back(iCurrentEdge);
							}
							edgesToEvaluate.push(currentEdge.nextEdge);
						}
					}
					else {
						// 1.3.2. Continue searching in the current HEFace
						edgesToEvaluate.push(currentEdge.nextEdge);
					}
				}
			}
			while (edgesToEvaluate.top() != initialFace.edge);
		}

		return std::make_pair(horizonEdges, visibleFaces);
	}

}
