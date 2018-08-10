#include <set>
#include <limits>
#include <algorithm>
#include "fe/collision/HalfEdgeMesh.h"

namespace fe { namespace collision {

	int addVertex(HalfEdgeMesh& meshData, const glm::vec3& point)
	{
		int iVertex = meshData.vertices.create();
		meshData.vertices[iVertex].location = point;

		return iVertex;
	}


	void removeVertex(HalfEdgeMesh& meshData, int iVertex)
	{
		if (!meshData.edges.isActive(iVertex)) { return; }

		int iInitialEdge = meshData.vertices[iVertex].edge;
		int iCurrentEdge = iInitialEdge;
		do {
			HEEdge& currentEdge = meshData.edges[iCurrentEdge];
			iCurrentEdge = currentEdge.oppositeEdge;
			removeFace(meshData, currentEdge.face);
		}
		while (iCurrentEdge != iInitialEdge);

		meshData.vertices.free(iVertex);
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
			while ((iCurrentEdge != iInitialEdge) && (iCurrentEdge >= 0));

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


	int addFace(HalfEdgeMesh& meshData, const std::vector<int>& vertexIndices)
	{
		if (vertexIndices.size() < 3) { return -1; }

		// Create a new HEFace
		int iFace = meshData.faces.create();

		// Create or set the face and vertex edges
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

			// Set the HEFace of the HEEdge
			meshData.edges[iEdge1].face = iFace;
		}

		// Set the previous and next edges of the face edges
		for (std::size_t i = 0; i < edgeIndices.size(); ++i) {
			int iCurrentEdge	= edgeIndices[i];
			int iPreviousEdge	= edgeIndices[(i == 0)? edgeIndices.size() - 1 : i - 1];
			int iNextEdge		= edgeIndices[(i + 1) % edgeIndices.size()];

			HEEdge& currentEdge = meshData.edges[iCurrentEdge];
			currentEdge.previousEdge	= iPreviousEdge;
			currentEdge.nextEdge		= iNextEdge;
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
			HEEdge& currentEdge	= meshData.edges[iCurrentEdge];
			int iNextEdge		= currentEdge.nextEdge;
			int iOppositeEdge	= currentEdge.oppositeEdge;
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
				meshData.edges.free(iCurrentEdge);
				meshData.edges.free(iOppositeEdge);
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
			for (const auto& pair : meshData.vertexEdgeMap) {
				if (pair.first.first == iCurrentVertex) {
					meshData.vertices[iCurrentVertex].edge = pair.second;
					break;
				}
			}
		}

		meshData.faces.free(iFace);
	}


	std::vector<int> getFaceIndices(const HalfEdgeMesh& meshData, int iFace)
	{
		std::vector<int> indices;

		int initialEdgeIndex = meshData.faces[iFace].edge;
		int currentEdgeIndex = initialEdgeIndex;
		do {
			const HEEdge& currentEdge	= meshData.edges[currentEdgeIndex];
			const HEEdge& oppositeEdge	= meshData.edges[currentEdge.oppositeEdge];
			indices.push_back(oppositeEdge.vertex);
			currentEdgeIndex = currentEdge.nextEdge;
		}
		while (currentEdgeIndex != initialEdgeIndex);

		return indices;
	}


	int mergeFaces(HalfEdgeMesh& meshData, int iFace1, int iFace2)
	{
		if (!meshData.faces.isActive(iFace1) || !meshData.faces.isActive(iFace2)
			|| (iFace1 == iFace2)
		) {
			return -1;
		}

		// Get an HEEdge of an HEEdge loop section which belongs to the HEFace
		// 2 and that is going to be preserved in the joined HEFace
		bool found = false;
		int iCurrentEdge = meshData.faces[iFace2].edge;
		int iInitialEdge = iCurrentEdge;
		do {
			const HEEdge& currentEdge = meshData.edges[iCurrentEdge];
			if (meshData.edges[currentEdge.oppositeEdge].face != iFace1) {
				found = true;
			}
			else {
				iCurrentEdge = currentEdge.nextEdge;
			}
		}
		while ((iCurrentEdge != iInitialEdge) && !found);

		if (!found) {
			// The HEFaces doesn't share any HEEdge, return
			return -1;
		}

		// Join the loops of HEEdges between both HEFaces
		bool newSection = true;
		iInitialEdge = iCurrentEdge;
		do {
			HEEdge& currentEdge = meshData.edges[iCurrentEdge];
			HEEdge& oppositeEdge = meshData.edges[currentEdge.oppositeEdge];
			int iNextEdge = currentEdge.nextEdge;

			// Update the current HEEdge's HEFace
			currentEdge.face = iFace1;

			// Check if the current HEEdge is shared between the HEFaces to
			// merge
			if (oppositeEdge.face == iFace1) {
				// Join the bounds of the HEEdge loop
				if (newSection) {
					meshData.edges[currentEdge.previousEdge].nextEdge	= oppositeEdge.nextEdge;
					meshData.edges[oppositeEdge.nextEdge].previousEdge	= currentEdge.previousEdge;
				}
				meshData.edges[currentEdge.nextEdge].previousEdge	= oppositeEdge.previousEdge;
				meshData.edges[oppositeEdge.previousEdge].nextEdge	= currentEdge.nextEdge;

				// Update the HEVertices' HEEdges
				HEVertex& currentVertex = meshData.vertices[currentEdge.vertex];
				HEVertex& oppositeVertex = meshData.vertices[oppositeEdge.vertex];
				if (currentVertex.edge == currentEdge.oppositeEdge) {
					currentVertex.edge = currentEdge.nextEdge;
				}
				if (oppositeVertex.edge == iCurrentEdge) {
					oppositeVertex.edge = oppositeEdge.nextEdge;
				}

				// Remove both HEEdges
				meshData.vertexEdgeMap.erase(std::make_pair(oppositeEdge.vertex, currentEdge.vertex));
				meshData.vertexEdgeMap.erase(std::make_pair(currentEdge.vertex, oppositeEdge.vertex));
				meshData.edges.free(currentEdge.oppositeEdge);
				meshData.edges.free(iCurrentEdge);

				newSection = false;
			}
			else {
				newSection = true;
			}

			iCurrentEdge = iNextEdge;
		}
		while (iCurrentEdge != iInitialEdge);

		// Update the first HEFace's HEEdge
		meshData.faces[iFace1].edge = iCurrentEdge;

		// Remove the second HEFace
		meshData.faces.free(iFace2);

		return iFace1;
	}


	void triangulateFaces(HalfEdgeMesh& meshData)
	{
		HalfEdgeMesh triangulatedMesh;
		triangulatedMesh.vertices = meshData.vertices;

		for (const HEFace& face : meshData.faces) {
			HEEdge currentEdge	= meshData.edges[face.edge];
			HEEdge oppositeEdge	= meshData.edges[currentEdge.oppositeEdge];

			int initialVertexIndex	= oppositeEdge.vertex;
			int lastEdgeIndex		= currentEdge.previousEdge;
			int currentEdgeIndex	= currentEdge.nextEdge;
			do {
				currentEdge		= meshData.edges[currentEdgeIndex];
				oppositeEdge	= meshData.edges[currentEdge.oppositeEdge];
				addFace(triangulatedMesh, { initialVertexIndex, oppositeEdge.vertex, currentEdge.vertex });
				currentEdgeIndex = currentEdge.nextEdge;
			}
			while (currentEdgeIndex != lastEdgeIndex);
		}

		meshData = triangulatedMesh;
	}


	glm::vec3 calculateFaceNormal(const HalfEdgeMesh& meshData, int iFace)
	{
		glm::vec3 normal(0.0f);

		int initialEdgeIndex = meshData.faces[iFace].edge;
		int currentEdgeIndex = initialEdgeIndex;
		do {
			const HEEdge& currentEdge = meshData.edges[currentEdgeIndex];
			const HEEdge& nextEdge	= meshData.edges[currentEdge.nextEdge];
			glm::vec3 currentVertex	= meshData.vertices[currentEdge.vertex].location;
			glm::vec3 nextVertex	= meshData.vertices[nextEdge.vertex].location;

			normal.x += (currentVertex.y - nextVertex.y) * (currentVertex.z + nextVertex.z);
			normal.y += (currentVertex.z - nextVertex.z) * (currentVertex.x + nextVertex.x);
			normal.z += (currentVertex.x - nextVertex.x) * (currentVertex.y + nextVertex.y);

			currentEdgeIndex = currentEdge.nextEdge;
		}
		while (currentEdgeIndex != initialEdgeIndex);

		return glm::normalize(normal);
	}


	std::pair<std::vector<int>, std::vector<int>> calculateHorizon(
		const HalfEdgeMesh& meshData, const std::map<int, glm::vec3>& faceNormals,
		const glm::vec3& eyePoint, int iFace
	) {
		std::vector<int> horizonEdges, visibleFaces;

		// Test the visibility of the initial HEFace
		HEFace inputFace = meshData.faces[iFace];
		HEVertex inputFaceVertex = meshData.vertices[ meshData.edges[inputFace.edge].vertex ];
		glm::vec3 inputFaceNormal = faceNormals.find(iFace)->second;
		if (glm::dot(eyePoint - inputFaceVertex.location, inputFaceNormal) > 0) {
			visibleFaces.push_back(iFace);

			std::set<int> visitedFaces;
			int iInitialEdge = inputFace.edge;
			int iCurrentEdge = iInitialEdge;
			do {
				// 1. Mark the current face as visited
				HEEdge currentEdge = meshData.edges[iCurrentEdge];
				visitedFaces.insert(currentEdge.face);

				// 2. Get the next face as the one found by crossing the
				// currentEdge, more specifically, the HEFace of the opposite
				// HEEdge
				HEEdge oppositeEdge = meshData.edges[currentEdge.oppositeEdge];
				int iNextFace = oppositeEdge.face;

				// 3. Check if we already visited the next HEFace
				if ((iNextFace < 0)
					|| (std::find(visitedFaces.begin(), visitedFaces.end(), iNextFace) != visitedFaces.end())
				) {
					// 3.1. Check if we are in the returning step of the search
					if (!horizonEdges.empty() && (oppositeEdge.nextEdge >= 0)) {
						// 3.1.1. Continue with the next HEEdge of the next
						// HEFace
						iCurrentEdge = oppositeEdge.nextEdge;
					}
					else {
						// 3.1.2. Continue with the next HEEdge
						iCurrentEdge = currentEdge.nextEdge;
					}
				}
				else {
					// 3.2. Test the visibility of the next HEFace from the eye
					// point
					HEVertex nextFaceVertex = meshData.vertices[oppositeEdge.vertex];
					glm::vec3 nextFaceNormal = faceNormals.find(iNextFace)->second;
					if (glm::dot(eyePoint - nextFaceVertex.location, nextFaceNormal) > 0) {
						// 3.2.1. Mark the HEFace as visible and continue with
						// next HEEdge of the next HEFace
						visibleFaces.push_back(iNextFace);
						iCurrentEdge = oppositeEdge.nextEdge;
					}
					else {
						// 3.2.2. Mark the current HEEdge as an horizon HEEdge
						// and continue with the next HEEdge
						horizonEdges.push_back(iCurrentEdge);
						iCurrentEdge = currentEdge.nextEdge;
					}
				}
			}
			while (iCurrentEdge != iInitialEdge);
		}

		return std::make_pair(horizonEdges, visibleFaces);
	}

}}
