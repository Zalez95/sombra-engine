#include <array>
#include <algorithm>
#include "MeshAdjacencyData.h"

namespace fe { namespace collision {

	void MeshAdjacencyData::addVertex(const glm::vec3& point)
	{
		mVertices.emplace_back(point);
	}


	/*void MeshAdjacencyData::removeVertex(int index)
	{
		int initialEdgeIndex = vertices[index].edge;
		int currentEdgeIndex = initialEdgeIndex;
		do {
			Edge& currentEdge = edges[currentEdgeIndex];
			removeFace(currentEdge.face);
			currentEdgeIndex = currentEdge.oppositeEdge;
		}
		while (currentEdgeIndex != initialEdgeIndex);

		vertices.erase(vertices.begin() + index);
	}*/


	void MeshAdjacencyData::addFace(std::vector<int> vertexIndexes)
	{
		// Create a new face
		mFaces.emplace_back();
		int iFace = mFaces.size() - 1;

		// Create or set the face and vertex edges
		std::vector<int> edgeIndexes;
		for (size_t i = 0; i < vertexIndexes.size(); ++i) {
			int iVertex1 = vertexIndexes[i],
				iVertex2 = vertexIndexes[(i + 1) % vertexIndexes.size()];

			// Get the Edge of the current vertices
			int iEdge;
			auto edgeIt = mVertexEdgeMap.find(std::make_pair(iVertex1, iVertex2));
			if (edgeIt != mVertexEdgeMap.end()) {
				iEdge = edgeIt->second;
			}
			else {
				// Create a new Edge
				mEdges.emplace_back();
				int iEdge1 = mEdges.size() - 1;

				// Create the oposite edge
				mEdges.emplace_back();
				int iEdge2 = mEdges.size() - 1;

				// Set the vertices of both edges
				mEdges[iEdge1].vertex = iVertex2;
				mEdges[iEdge2].vertex = iVertex1;

				// Set the opposite edge of both edges
				mEdges[iEdge1].oppositeEdge = iEdge2;
				mEdges[iEdge2].oppositeEdge = iEdge1;

				// Register the edges on the map
				mVertexEdgeMap[std::make_pair(iVertex2, iVertex2)] = iEdge1;
				mVertexEdgeMap[std::make_pair(iVertex2, iVertex1)] = iEdge2;

				iEdge = iEdge1;
			}
			edgeIndexes.push_back(iEdge);

			// Set the Edge of the Face
			if (mFaces[iFace].edge < 0) {
				mFaces[iFace].edge = iEdge;
			}

			// Set the Edge of the first Vertex
			if (mVertices[iVertex1].edge < 0) {
				mVertices[iVertex1].edge = iEdge;
			}

			// Set the Face of the Edge
			mEdges[iEdge].face = iFace;
		}

		// Set the previous and next edges of the face edges
		for (size_t iEdge = 0; iEdge < edgeIndexes.size(); ++iEdge) {
			int iPreviousEdge	= (iEdge == 0)? edgeIndexes.back() : edgeIndexes[iEdge - 1];
			int iNextEdge		= (iEdge + 1) % edgeIndexes.size();

			Edge& currentEdge = mEdges[edgeIndexes[iEdge]];
			currentEdge.previousEdge	= edgeIndexes[iPreviousEdge];
			currentEdge.nextEdge		= edgeIndexes[iNextEdge];
		}
	}


	/*void MeshAdjacencyData::removeFace(int index)
	{
		int initialEdgeIndex = faces[index].edge;
		int currentEdgeIndex = initialEdgeIndex;
		do {
			Edge& currentEdge	= edges[currentEdgeIndex];
			Edge& oppositeEdge	= edges[currentEdge.oppositeEdge];

			// Remove the currentEdge and its opposite one only if the
			// opposite Edge also has no Face
			if (oppositeEdge.face < 0) {
				
			}

			currentEdgeIndex
		}
		while (currentEdgeIndex != initialEdgeIndex);

		faces.erase(faces.begin() + index);
	}*/


	glm::vec3 calculateFaceNormal(int iFace, const MeshAdjacencyData& meshData)
	{
		glm::vec3 normal(0.0f);

		int initialEdgeIndex = meshData.getFace(iFace).edge;
		int currentEdgeIndex = initialEdgeIndex;
		do {
			const Edge& currentEdge = meshData.getEdge(currentEdgeIndex);
			const Edge& nextEdge	= meshData.getEdge(currentEdge.nextEdge);
			glm::vec3 currentVertex	= meshData.getVertex(currentEdge.vertex).location;
			glm::vec3 nextVertex	= meshData.getVertex(nextEdge.vertex).location;

			normal.x += (currentVertex.y - nextVertex.y) * (currentVertex.z + nextVertex.z);
			normal.y += (currentVertex.z - nextVertex.z) * (currentVertex.x + nextVertex.x);
			normal.z += (currentVertex.x - nextVertex.x) * (currentVertex.y + nextVertex.y);

			currentEdgeIndex = currentEdge.nextEdge;
		}
		while (currentEdgeIndex != initialEdgeIndex);

		return glm::normalize(normal);
	}


	std::vector<int> calculateHorizon(
		const glm::vec3& eyePoint,
		int iFace, const MeshAdjacencyData& meshData
	) {
		std::vector<int> visitedFaces;
		std::vector<int> horizonEdges; 

		int iInitialEdge = meshData.getFace(iFace).edge;
		int iCurrentEdge = iInitialEdge;
		do {
			// 1. Mark the current face as visited
			Edge currentEdge = meshData.getEdge(iCurrentEdge);
			visitedFaces.push_back(currentEdge.face);

			// 2. Get the next face as the one found by crossing the
			// currentEdge, more specifically, the Face of the opposite Edge
			Edge oppositeEdge = meshData.getEdge(currentEdge.oppositeEdge);
			int iNextFace = oppositeEdge.face;

			// 3. Check if we already visited the next Face
			if (std::any_of(visitedFaces.begin(), visitedFaces.end(), [&](int f) { return f == iNextFace; })) {
				// 4.1. Already visited, continue with the next Edge
				iCurrentEdge = currentEdge.nextEdge;
			}
			else {
				// 4.2. Test the visibility of the next face from the eye point
				Vertex nextFaceVertex = meshData.getVertex(oppositeEdge.vertex);
				glm::vec3 nextFaceNormal = calculateFaceNormal(iNextFace, meshData);
				if (glm::dot(eyePoint - nextFaceVertex.location, nextFaceNormal) >= 0) {
					// 4.3.1. Visible, continue with next Edge of the next Face
					iCurrentEdge = oppositeEdge.nextEdge;
				}
				else {
					// 4.3.2. Add the current Edge to the horizon list and
					// continue with the next Edge
					horizonEdges.push_back(iCurrentEdge);
					iCurrentEdge = currentEdge.nextEdge;
				}
			}
		}
		while (iCurrentEdge != iInitialEdge);

		return horizonEdges;
	}

}}
