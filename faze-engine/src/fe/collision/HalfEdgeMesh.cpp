#include <array>
#include <limits>
#include <cassert>
#include <algorithm>
#include "fe/collision/HalfEdgeMesh.h"

namespace fe { namespace collision {

	int HalfEdgeMesh::addVertex(const glm::vec3& point)
	{
		int iVertex = mVertices.create();
		mVertices[iVertex].location = point;

		return iVertex;
	}


	void HalfEdgeMesh::removeVertex(int i)
	{
		int iInitialEdge = mVertices[i].edge;
		int iCurrentEdge = iInitialEdge;
		do {
			Edge& currentEdge = mEdges[iCurrentEdge];
			iCurrentEdge = currentEdge.oppositeEdge;
			removeFace(currentEdge.face);
		}
		while (iCurrentEdge != iInitialEdge);

		mVertices.free(i);
	}


	int HalfEdgeMesh::addFace(const std::vector<int>& vertexIndexes)
	{
		// Create a new face
		int iFace = mFaces.create();

		// Create or set the face and vertex edges
		std::vector<int> edgeIndexes;
		for (size_t i = 0; i < vertexIndexes.size(); ++i) {
			int iVertex1 = vertexIndexes[i],
				iVertex2 = vertexIndexes[(i + 1) % vertexIndexes.size()];

			// Get the Edge of the current vertices
			int iEdge1, iEdge2;
			auto edgeIt = mVertexEdgeMap.find(std::make_pair(iVertex1, iVertex2));
			if (edgeIt != mVertexEdgeMap.end()) {
				iEdge1 = edgeIt->second;
				iEdge2 = mEdges[iEdge1].oppositeEdge;
			}
			else {
				// Create a new Edge
				iEdge1 = mEdges.create();

				// Create the oposite edge
				iEdge2 = mEdges.create();

				// Set the vertices of both edges
				mEdges[iEdge1].vertex = iVertex2;
				mEdges[iEdge2].vertex = iVertex1;

				// Set the opposite edge of both edges
				mEdges[iEdge1].oppositeEdge = iEdge2;
				mEdges[iEdge2].oppositeEdge = iEdge1;

				// Register the edges on the map
				mVertexEdgeMap[std::make_pair(iVertex1, iVertex2)] = iEdge1;
				mVertexEdgeMap[std::make_pair(iVertex2, iVertex1)] = iEdge2;
			}

			// Set the Edge of the Face
			edgeIndexes.push_back(iEdge1);
			if (mFaces[iFace].edge < 0) {
				mFaces[iFace].edge = iEdge1;
			}

			// Set the Edge of the first Vertex
			if (mVertices[iVertex1].edge < 0) {
				mVertices[iVertex1].edge = iEdge2;
			}

			// Set the Face of the Edge
			mEdges[iEdge1].face = iFace;
		}

		// Set the previous and next edges of the face edges
		for (size_t i = 0; i < edgeIndexes.size(); ++i) {
			int iPreviousEdge	= (i == 0)? edgeIndexes.back() : edgeIndexes[i - 1];
			int iNextEdge		= edgeIndexes[(i + 1) % edgeIndexes.size()];

			Edge& currentEdge = mEdges[ edgeIndexes[i] ];
			currentEdge.previousEdge	= iPreviousEdge;
			currentEdge.nextEdge		= iNextEdge;
		}

		return iFace;
	}


	void HalfEdgeMesh::removeFace(int i)
	{
		std::vector<int> vertexIndices;

		int iInitialEdge = mFaces[i].edge;
		int iCurrentEdge = iInitialEdge;
		do {
			Edge& currentEdge	= mEdges[iCurrentEdge];
			int iNextEdge		= currentEdge.nextEdge;
			int iOppositeEdge	= currentEdge.oppositeEdge;
			Edge& oppositeEdge	= mEdges[iOppositeEdge];
			vertexIndices.push_back(currentEdge.vertex);

			// Remove the currentEdge and its opposite one only if the
			// opposite Edge hasn't a Face
			if (oppositeEdge.face < 0) {
				int iVertex1 = oppositeEdge.vertex;
				int iVertex2 = currentEdge.vertex;

				// Remove the Edges from the map
				auto itEdge1 = mVertexEdgeMap.find(std::make_pair(iVertex1, iVertex2));
				if (itEdge1 != mVertexEdgeMap.end()) {
					mVertexEdgeMap.erase(itEdge1);
				}

				auto itEdge2 = mVertexEdgeMap.find(std::make_pair(iVertex2, iVertex1));
				if (itEdge2 != mVertexEdgeMap.end()) {
					mVertexEdgeMap.erase(itEdge2);
				}

				// Remove the Edges
				mEdges.free(iCurrentEdge);
				mEdges.free(iOppositeEdge);
			}

			iCurrentEdge = iNextEdge;
		}
		while (iCurrentEdge != iInitialEdge);

		// Update the Edges of the Face Vertices
		for (int iCurrentVertex : vertexIndices) {
			for (const auto& pair : mVertexEdgeMap) {
				if (pair.first.first == iCurrentEdge) {
					mVertices[iCurrentVertex].edge = pair.second;
					break;
				}
			}
		}

		mFaces.free(i);
	}


	void HalfEdgeMesh::mergeFace(int iEdge)
	{
		Edge currentEdge	= mEdges[iEdge];
		Edge oppositeEdge	= mEdges[currentEdge.oppositeEdge];

		// Remove the opposite face
		currentEdge.face = -1;
		removeFace(oppositeEdge.face);

		// Join the edges of both Faces by the current Edge position
		mEdges[oppositeEdge.previousEdge].nextEdge	= currentEdge.nextEdge;
		mEdges[currentEdge.nextEdge].previousEdge	= oppositeEdge.previousEdge;
		mEdges[currentEdge.previousEdge].nextEdge	= oppositeEdge.nextEdge;
		mEdges[oppositeEdge.nextEdge].previousEdge	= currentEdge.previousEdge;

		// Update the Vertices' Edges
		Vertex& currentVertex = mVertices[currentEdge.vertex];
		Vertex& oppositeVertex = mVertices[oppositeEdge.vertex];
		if (currentVertex.edge == currentEdge.oppositeEdge) {
			currentVertex.edge = currentEdge.nextEdge;
		}
		if (oppositeVertex.edge == iEdge) {
			oppositeVertex.edge = oppositeEdge.nextEdge;
		}

		// Update the updated Face's Edge
		Face& currentFace = mFaces[currentEdge.face];
		if (currentFace.edge == iEdge) {
			currentFace.edge = currentEdge.nextEdge;
		}
	}


	glm::vec3 calculateFaceNormal(int iFace, const HalfEdgeMesh& meshData)
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


	int getFurthestVertexInDirection(
		const glm::vec3& direction,
		const HalfEdgeMesh& meshData
	) {
		auto getVertexDistance = [](const glm::vec3& location, const glm::vec3& direction) {
			return glm::dot(location, direction);
		};

		int iBestEdge = 0;
		float bestDistance = getVertexDistance(
			meshData.getVertex( meshData.getEdge(iBestEdge).vertex ).location,
			direction
		);

		for (bool end = false; !end;) {
			// Search the best neighbour of the current vertex
			int iInitialEdge = iBestEdge, iCurrentEdge = iBestEdge, iBestEdge2 = iBestEdge;
			float bestDistance2 = bestDistance;
			do {
				Edge currentEdge = meshData.getEdge(iCurrentEdge);
				Vertex currentVertex = meshData.getVertex(currentEdge.vertex);

				float currentDistance = getVertexDistance(currentVertex.location, direction);
				if (currentDistance > bestDistance2) {
					bestDistance2 = currentDistance;
					iBestEdge2 = iCurrentEdge;
				}

				iCurrentEdge = meshData.getEdge(currentEdge.oppositeEdge).nextEdge;
			}
			while (iCurrentEdge != iInitialEdge);

			if (bestDistance2 > bestDistance) {
				// Update our upper bound
				bestDistance = bestDistance2;
				iBestEdge = iBestEdge2;
			}
			else {
				// Found maximum
				end = true;
			}
		}

		return meshData.getEdge(iBestEdge).vertex;
	}


	std::pair<std::vector<int>, std::vector<int>> calculateHorizon(
		const glm::vec3& eyePoint,
		int iFace, const HalfEdgeMesh& meshData
	) {
		assert((iFace >= 0) && (iFace < meshData.getNumFaces()) && "The index of the Face must be in range");

		std::vector<int> visitedFaces, horizonEdges, visibleFaces = { iFace };

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

			// 3. Check if the next face exists or if we already visited the
			// next Face
			if ((iNextFace < 0)
				|| std::any_of(visitedFaces.begin(), visitedFaces.end(), [&](int f) { return f == iNextFace; })
			) {
				// 4.1. Continue with the next Edge
				iCurrentEdge = currentEdge.nextEdge;
			}
			else {
				// 4.2. Test the visibility of the next face from the eye point
				Vertex nextFaceVertex = meshData.getVertex(oppositeEdge.vertex);
				glm::vec3 nextFaceNormal = calculateFaceNormal(iNextFace, meshData);
				if (glm::dot(eyePoint - nextFaceVertex.location, nextFaceNormal) >= 0) {
					// 4.3.1. Visible, continue with next Edge of the next Face
					visibleFaces.push_back(iNextFace);
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

		return std::make_pair(horizonEdges, visibleFaces);
	}

}}
