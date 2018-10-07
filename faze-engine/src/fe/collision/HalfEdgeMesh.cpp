#include <stack>
#include <limits>
#include <algorithm>
#include "fe/collision/AABB.h"
#include "fe/collision/HalfEdgeMesh.h"
#include "Geometry.h"

#define NORMALIZATION_ZERO 0.0001f

namespace fe { namespace collision {

	int addVertex(HalfEdgeMesh& meshData, const glm::vec3& point)
	{
		return meshData.vertices.create(point);
	}


	void removeVertex(HalfEdgeMesh& meshData, int iVertex)
	{
		if (!meshData.edges.isActive(iVertex)) { return; }

		int iInitialEdge = meshData.vertices[iVertex].edge;
		int iCurrentEdge = iInitialEdge;
		do {
			const HEEdge& currentEdge = meshData.edges[iCurrentEdge];
			const HEEdge& oppositeEdge = meshData.edges[currentEdge.oppositeEdge];

			if (meshData.faces.isActive(currentEdge.face)) {
				removeFace(meshData, currentEdge.face);
			}

			iCurrentEdge = oppositeEdge.nextEdge;
		}
		while ((iCurrentEdge != iInitialEdge) && meshData.edges.isActive(iCurrentEdge));

		meshData.vertices.release(iVertex);
	}


	int addFace(HalfEdgeMesh& meshData, const std::vector<int>& vertexIndices)
	{
		if (vertexIndices.size() < 3) { return -1; }

		// Create a new HEFace
		int iFace = meshData.faces.create();

		// Set the HEFace and HEVertices data and recover all the HEEdges
		std::vector<int> edgeIndices;
		for (std::size_t i = 0; i < vertexIndices.size(); ++i) {
			int iVertex1 = vertexIndices[i],
				iVertex2 = vertexIndices[(i + 1) % vertexIndices.size()];

			// Get the HEEdge of the current vertices
			int iEdge1, iEdge2;
			auto edgeIt = meshData.vertexEdgeMap.find(std::make_pair(iVertex1, iVertex2));
			if (edgeIt != meshData.vertexEdgeMap.end()) {
				iEdge1 = edgeIt->second;
				iEdge2 = meshData.edges[iEdge1].oppositeEdge;
			}
			else {
				// Create a new HEEdge
				iEdge1 = meshData.edges.create();

				// Create the opposite edge
				iEdge2 = meshData.edges.create();

				// Set the vertices of both HEEdges
				meshData.edges[iEdge1].vertex = iVertex2;
				meshData.edges[iEdge2].vertex = iVertex1;

				// Set the opposite edge of both HEEdges
				meshData.edges[iEdge1].oppositeEdge = iEdge2;
				meshData.edges[iEdge2].oppositeEdge = iEdge1;

				// Register the edges on the map
				meshData.vertexEdgeMap[std::make_pair(iVertex1, iVertex2)] = iEdge1;
				meshData.vertexEdgeMap[std::make_pair(iVertex2, iVertex1)] = iEdge2;
			}

			// Set the HEEdge of the HEFace
			edgeIndices.push_back(iEdge1);
			if (meshData.faces[iFace].edge < 0) {
				meshData.faces[iFace].edge = iEdge1;
			}

			// Set the HEEdge of the first HEVertex
			if (meshData.vertices[iVertex1].edge < 0) {
				meshData.vertices[iVertex1].edge = iEdge1;
			}
		}

		// Set the previous and next HEEdges of the HEFace's HEEdges
		for (std::size_t i = 0; i < edgeIndices.size(); ++i) {
			HEEdge& currentEdge = meshData.edges[ edgeIndices[i] ];
			currentEdge.face			= iFace;
			currentEdge.previousEdge	= edgeIndices[(i == 0)? edgeIndices.size() - 1 : i - 1];
			currentEdge.nextEdge		= edgeIndices[(i + 1) % edgeIndices.size()];
		}

		return iFace;
	}


	void removeFace(HalfEdgeMesh& meshData, int iFace)
	{
		if (!meshData.faces.isActive(iFace)) { return; }

		std::vector<int> vertexIndices;

		int iInitialEdge = meshData.faces[iFace].edge;
		int iCurrentEdge = iInitialEdge;
		do {
			HEEdge& currentEdge		= meshData.edges[iCurrentEdge];
			int iNextEdge			= currentEdge.nextEdge;
			int iOppositeEdge		= currentEdge.oppositeEdge;
			HEEdge& oppositeEdge	= meshData.edges[iOppositeEdge];
			vertexIndices.push_back(currentEdge.vertex);

			// Remove the currentEdge and its opposite one only if the
			// opposite HEEdge hasn't a HEFace
			if (oppositeEdge.face < 0) {
				int iVertex1 = oppositeEdge.vertex;
				int iVertex2 = currentEdge.vertex;

				// Remove the Edges from the map
				meshData.vertexEdgeMap.erase(std::make_pair(iVertex1, iVertex2));
				meshData.vertexEdgeMap.erase(std::make_pair(iVertex2, iVertex1));

				// Remove the Edges
				meshData.edges.release(iCurrentEdge);
				meshData.edges.release(iOppositeEdge);
			}
			else {
				// Reset the HEFace data of the current HEEdge
				currentEdge.face			= -1;
				currentEdge.previousEdge	= -1;
				currentEdge.nextEdge		= -1;
			}

			iCurrentEdge = iNextEdge;
		}
		while (iCurrentEdge != iInitialEdge);

		// Update the HEVertices of the HEFace
		for (int iCurrentVertex : vertexIndices) {
			// Search an HEEdge that starts with the current HEVertex
			int newVertexEdge = -1;
			for (const auto& pair : meshData.vertexEdgeMap) {
				if (pair.first.first == iCurrentVertex) {
					newVertexEdge = pair.second;
					break;
				}
			}

			meshData.vertices[iCurrentVertex].edge = newVertexEdge;
		}

		meshData.faces.release(iFace);
	}


	int mergeFaces(HalfEdgeMesh& meshData, int iFace1, int iFace2)
	{
		int iFace = -1;

		if (iFace1 == iFace2) {
			iFace = iFace1;
		}
		else if (meshData.faces.isActive(iFace1) && meshData.faces.isActive(iFace2)) {
			// Find the HEEdge loop sections shared between both HEFaces
			struct Section { int iInitialEdge, iFinalEdge, length; };
			std::vector<Section> sections;

			int iInitialEdge = meshData.faces[iFace1].edge;
			int iCurrentEdge = iInitialEdge;
			do {
				const HEEdge& currentEdge = meshData.edges[iCurrentEdge];
				if (meshData.edges[currentEdge.oppositeEdge].face == iFace2) {
					if (sections.empty() || (sections.back().iFinalEdge != currentEdge.previousEdge)) {
						sections.push_back({ iCurrentEdge, iCurrentEdge, 1 });
					}
					else {
						Section& currentSection = sections.back();
						currentSection.iFinalEdge = iCurrentEdge;
						currentSection.length++;
					}
				}

				iCurrentEdge = currentEdge.nextEdge;
			}
			while (iCurrentEdge != iInitialEdge);

			// Merge the faces by the longest shared section
			auto itSection = std::max_element(
				sections.begin(), sections.end(),
				[](const Section& s1, const Section& s2) { return s1.length > s2.length; }
			);
			if (itSection != sections.end()) {
				// Close the new HEEdge loop of the first HEFace
				const HEEdge& initialEdge = meshData.edges[itSection->iInitialEdge];
				const HEEdge& oppositeInitialEdge = meshData.edges[initialEdge.oppositeEdge];
				meshData.edges[initialEdge.previousEdge].nextEdge = oppositeInitialEdge.nextEdge;
				meshData.edges[oppositeInitialEdge.nextEdge].previousEdge = initialEdge.previousEdge;

				const HEEdge& finalEdge = meshData.edges[itSection->iFinalEdge];
				const HEEdge& oppositeFinalEdge = meshData.edges[finalEdge.oppositeEdge];
				meshData.edges[finalEdge.nextEdge].previousEdge = oppositeFinalEdge.previousEdge;
				meshData.edges[oppositeFinalEdge.previousEdge].nextEdge = finalEdge.nextEdge;

				// Update HEEdge of the first HEFace
				meshData.faces[iFace1].edge = initialEdge.previousEdge;

				// Set the first HEFace of the first HEFace's HEEdges
				iInitialEdge = meshData.faces[iFace1].edge;
				iCurrentEdge = iInitialEdge;
				do {
					HEEdge& currentEdge = meshData.edges[iCurrentEdge];
					currentEdge.face = iFace1;
					iCurrentEdge = currentEdge.nextEdge;
				}
				while (iCurrentEdge != iInitialEdge);

				// Remove the shared HEEdge section
				iCurrentEdge = itSection->iInitialEdge;
				int iFinalEdge = finalEdge.nextEdge;
				while (iCurrentEdge != iFinalEdge) {
					const HEEdge& currentEdge	= meshData.edges[iCurrentEdge];
					int iNextEdge				= currentEdge.nextEdge;
					int iOppositeEdge			= currentEdge.oppositeEdge;
					const HEEdge& oppositeEdge	= meshData.edges[iOppositeEdge];

					int iVertex1 = oppositeEdge.vertex;
					int iVertex2 = currentEdge.vertex;

					// Remove the Edges from the map
					meshData.vertexEdgeMap.erase(std::make_pair(iVertex1, iVertex2));
					meshData.vertexEdgeMap.erase(std::make_pair(iVertex2, iVertex1));

					// Remove the Edges
					meshData.edges.release(iCurrentEdge);
					meshData.edges.release(iOppositeEdge);

					// Update the HEVertices' HEEdges
					int newVertexEdge = -1;
					for (const auto& pair : meshData.vertexEdgeMap) {
						if (pair.first.first == iVertex2) {
							newVertexEdge = pair.second;
							break;
						}
					}
					meshData.vertices[iVertex2].edge = newVertexEdge;

					iCurrentEdge = iNextEdge;
				}

				// Release the second HEFace
				meshData.faces.release(iFace2);

				iFace = iFace1;
			}
		}

		return iFace;
	}


	HalfEdgeMesh triangulateFaces(const HalfEdgeMesh& originalMesh)
	{
		HalfEdgeMesh triangulatedMesh;
		triangulatedMesh.vertices = originalMesh.vertices;

		for (const HEFace& face : originalMesh.faces) {
			const HEEdge& initialEdge = originalMesh.edges[face.edge];
			int iLastEdge		= initialEdge.previousEdge;
			int iCurrentEdge	= initialEdge.nextEdge;
			int iInitialVertex	= originalMesh.edges[ initialEdge.oppositeEdge ].vertex;
			do {
				const HEEdge& currentEdge	= originalMesh.edges[iCurrentEdge];
				const HEEdge& oppositeEdge	= originalMesh.edges[currentEdge.oppositeEdge];
				addFace(triangulatedMesh, { iInitialVertex, oppositeEdge.vertex, currentEdge.vertex });
				iCurrentEdge = currentEdge.nextEdge;
			}
			while (iCurrentEdge != iLastEdge);
		}

		return triangulatedMesh;
	}


	std::vector<int> getFaceIndices(const HalfEdgeMesh& meshData, int iFace)
	{
		std::vector<int> indices;

		if (meshData.faces.isActive(iFace)) {
			int iInitialEdge = meshData.faces[iFace].edge;
			int iCurrentEdge = iInitialEdge;
			do {
				const HEEdge& currentEdge	= meshData.edges[iCurrentEdge];
				const HEEdge& oppositeEdge	= meshData.edges[currentEdge.oppositeEdge];
				indices.push_back(oppositeEdge.vertex);
				iCurrentEdge = currentEdge.nextEdge;
			}
			while ((iCurrentEdge != iInitialEdge) && meshData.edges.isActive(iCurrentEdge));
		}

		return indices;
	}


	glm::vec3 calculateVertexNormal(
		const HalfEdgeMesh& meshData, const NormalMap& faceNormals,
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
						normal += faceNormals.at(currentEdge.face);
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
							normal += faceNormals.at(currentEdge.face);
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
		const HalfEdgeMesh& meshData, const NormalMap& faceNormals,
		const glm::vec3& eyePoint, int iInitialFace
	) {
		std::vector<int> horizonEdges, visibleFaces;

		// Test the visibility of the initial HEFace
		const HEFace& initialFace = meshData.faces[iInitialFace];
		const HEVertex& initialFaceVertex = meshData.vertices[ meshData.edges[initialFace.edge].vertex ];
		const glm::vec3& initialFaceNormal = faceNormals.at(iInitialFace);
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
					const glm::vec3& oppositeFaceNormal = faceNormals.at(oppositeEdge.face);
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
							if (std::find(visibleFaces.begin(), visibleFaces.end(), oppositeEdge.face)
								== visibleFaces.end()
							) {
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

}}
