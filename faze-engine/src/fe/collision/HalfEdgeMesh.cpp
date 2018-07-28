#include <set>
#include <limits>
#include <algorithm>
#include "fe/collision/HalfEdgeMesh.h"

namespace fe { namespace collision {

	int HalfEdgeMesh::addVertex(const glm::vec3& point)
	{
		int iVertex = mVertices.create();
		mVertices[iVertex].location = point;

		return iVertex;
	}


	void HalfEdgeMesh::removeVertex(int iVertex)
	{
		if (!mEdges.isActive(iVertex)) { return; }

		int iInitialEdge = mVertices[iVertex].edge;
		int iCurrentEdge = iInitialEdge;
		do {
			HEEdge& currentEdge = mEdges[iCurrentEdge];
			iCurrentEdge = currentEdge.oppositeEdge;
			removeFace(currentEdge.face);
		}
		while (iCurrentEdge != iInitialEdge);

		mVertices.free(iVertex);
	}


	int HalfEdgeMesh::addFace(const std::vector<int>& vertexIndices)
	{
		if (vertexIndices.size() < 3) { return -1; }

		// Create a new HEFace
		int iFace = mFaces.create();

		// Create or set the face and vertex edges
		std::vector<int> edgeIndices;
		for (std::size_t i = 0; i < vertexIndices.size(); ++i) {
			int iVertex1 = vertexIndices[i],
				iVertex2 = vertexIndices[(i + 1) % vertexIndices.size()];

			// Get the HEEdge of the current vertices
			int iEdge1, iEdge2;
			auto edgeIt = mVertexEdgeMap.find(std::make_pair(iVertex1, iVertex2));
			if (edgeIt != mVertexEdgeMap.end()) {
				iEdge1 = edgeIt->second;
				iEdge2 = mEdges[iEdge1].oppositeEdge;
			}
			else {
				// Create a new HEEdge
				iEdge1 = mEdges.create();

				// Create the opposite edge
				iEdge2 = mEdges.create();

				// Set the vertices of both HEEdges
				mEdges[iEdge1].vertex = iVertex2;
				mEdges[iEdge2].vertex = iVertex1;

				// Set the opposite edge of both HEEdges
				mEdges[iEdge1].oppositeEdge = iEdge2;
				mEdges[iEdge2].oppositeEdge = iEdge1;

				// Register the edges on the map
				mVertexEdgeMap[std::make_pair(iVertex1, iVertex2)] = iEdge1;
				mVertexEdgeMap[std::make_pair(iVertex2, iVertex1)] = iEdge2;
			}

			// Set the HEEdge of the HEFace
			edgeIndices.push_back(iEdge1);
			if (mFaces[iFace].edge < 0) {
				mFaces[iFace].edge = iEdge1;
			}

			// Set the HEEdge of the first HEVertex
			if (mVertices[iVertex1].edge < 0) {
				mVertices[iVertex1].edge = iEdge1;
			}

			// Set the HEFace of the HEEdge
			mEdges[iEdge1].face = iFace;
		}

		// Set the previous and next edges of the face edges
		for (std::size_t i = 0; i < edgeIndices.size(); ++i) {
			int iCurrentEdge	= edgeIndices[i];
			int iPreviousEdge	= edgeIndices[(i == 0)? edgeIndices.size() - 1 : i - 1];
			int iNextEdge		= edgeIndices[(i + 1) % edgeIndices.size()];

			HEEdge& currentEdge = mEdges[iCurrentEdge];
			currentEdge.previousEdge	= iPreviousEdge;
			currentEdge.nextEdge		= iNextEdge;
		}

		return iFace;
	}


	void HalfEdgeMesh::removeFace(int iFace)
	{
		if (!mFaces.isActive(iFace)) { return; }

		std::vector<int> vertexIndices;

		int iInitialEdge = mFaces[iFace].edge;
		int iCurrentEdge = iInitialEdge;
		do {
			HEEdge& currentEdge	= mEdges[iCurrentEdge];
			int iNextEdge		= currentEdge.nextEdge;
			int iOppositeEdge	= currentEdge.oppositeEdge;
			HEEdge& oppositeEdge	= mEdges[iOppositeEdge];
			vertexIndices.push_back(currentEdge.vertex);

			// Remove the currentEdge and its opposite one only if the
			// opposite HEEdge hasn't a HEFace
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
			for (const auto& pair : mVertexEdgeMap) {
				if (pair.first.first == iCurrentVertex) {
					mVertices[iCurrentVertex].edge = pair.second;
					break;
				}
			}
		}

		mFaces.free(iFace);
	}


	void HalfEdgeMesh::mergeFace(int iEdge)
	{
		if (!mEdges.isActive(iEdge)) { return; }

		const HEEdge& currentEdge	= mEdges[iEdge];
		const HEEdge& oppositeEdge	= mEdges[currentEdge.oppositeEdge];

		int iMergedFace = currentEdge.face;

		// Join the edges of both Faces by the current HEEdge position
		mEdges[oppositeEdge.previousEdge].nextEdge	= currentEdge.nextEdge;
		mEdges[currentEdge.nextEdge].previousEdge	= oppositeEdge.previousEdge;
		mEdges[currentEdge.previousEdge].nextEdge	= oppositeEdge.nextEdge;
		mEdges[oppositeEdge.nextEdge].previousEdge	= currentEdge.previousEdge;

		// Update the HEFace of the HEEdges
		int iCurrentEdge = oppositeEdge.nextEdge;
		while (mEdges[iCurrentEdge].face != iMergedFace) {
			HEEdge& currentEdge2 = mEdges[iCurrentEdge];
			currentEdge2.face = iMergedFace;
			iCurrentEdge = currentEdge2.nextEdge;
		}

		// Update the joined HEFace's HEEdge
		HEFace& currentFace = mFaces[iMergedFace];
		if (currentFace.edge == iEdge) {
			currentFace.edge = currentEdge.nextEdge;
		}

		// Update the Vertices' Edges
		HEVertex& currentVertex = mVertices[currentEdge.vertex];
		HEVertex& oppositeVertex = mVertices[oppositeEdge.vertex];
		if (currentVertex.edge == currentEdge.oppositeEdge) {
			currentVertex.edge = currentEdge.nextEdge;
		}
		if (oppositeVertex.edge == iEdge) {
			oppositeVertex.edge = oppositeEdge.nextEdge;
		}

		// Remove the opposite face
		mFaces.free(oppositeEdge.face);

		// Remove the current and opposite HEEdges
		mEdges.free(currentEdge.oppositeEdge);
		mEdges.free(iEdge);
	}


	glm::vec3 calculateFaceNormal(int iFace, const HalfEdgeMesh& meshData)
	{
		glm::vec3 normal(0.0f);

		int initialEdgeIndex = meshData.getFace(iFace).edge;
		int currentEdgeIndex = initialEdgeIndex;
		do {
			const HEEdge& currentEdge = meshData.getEdge(currentEdgeIndex);
			const HEEdge& nextEdge	= meshData.getEdge(currentEdge.nextEdge);
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


	std::vector<int> getFaceIndices(int iFace, const HalfEdgeMesh& meshData)
	{
		std::vector<int> indices;

		int initialEdgeIndex = meshData.getFace(iFace).edge;
		int currentEdgeIndex = initialEdgeIndex;
		do {
			const HEEdge& currentEdge	= meshData.getEdge(currentEdgeIndex);
			const HEEdge& oppositeEdge	= meshData.getEdge(currentEdge.oppositeEdge);
			indices.push_back(oppositeEdge.vertex);
			currentEdgeIndex = currentEdge.nextEdge;
		}
		while (currentEdgeIndex != initialEdgeIndex);

		return indices;
	}


	int getFurthestVertexInDirection(
		const glm::vec3& direction,
		const HalfEdgeMesh& meshData
	) {
		auto getVertexDistance = [&direction](const glm::vec3& location) {
			return glm::dot(location, direction);
		};

		int iBestVertex = meshData.getVerticesVector().begin().getIndex();
		float bestDistance = getVertexDistance(meshData.getVertex(iBestVertex).location);

		for (bool end = false; !end;) {
			// Search the best neighbour of the current vertex
			int iInitialEdge = meshData.getVertex(iBestVertex).edge, iCurrentEdge = iInitialEdge,
				iBestVertex2 = -1;
			float bestDistance2 = -std::numeric_limits<float>::max();
			do {
				HEEdge currentEdge = meshData.getEdge(iCurrentEdge);
				HEVertex currentVertex = meshData.getVertex(currentEdge.vertex);

				float currentDistance = getVertexDistance(currentVertex.location);
				if (currentDistance > bestDistance2) {
					bestDistance2 = currentDistance;
					iBestVertex2 = currentEdge.vertex;
				}

				iCurrentEdge = meshData.getEdge(currentEdge.oppositeEdge).nextEdge;
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


	std::pair<std::vector<int>, std::vector<int>> calculateHorizon(
		const glm::vec3& eyePoint, int iFace,
		const HalfEdgeMesh& meshData, const std::map<int, glm::vec3>& faceNormals
	) {
		std::vector<int> horizonEdges, visibleFaces;

		// Test the visibility of the initial HEFace
		HEFace inputFace = meshData.getFace(iFace);
		HEVertex inputFaceVertex = meshData.getVertex( meshData.getEdge(inputFace.edge).vertex );
		glm::vec3 inputFaceNormal = faceNormals.find(iFace)->second;
		if (glm::dot(eyePoint - inputFaceVertex.location, inputFaceNormal) > 0) {
			visibleFaces.push_back(iFace);

			std::set<int> visitedFaces;
			int iInitialEdge = inputFace.edge;
			int iCurrentEdge = iInitialEdge;
			do {
				// 1. Mark the current face as visited
				HEEdge currentEdge = meshData.getEdge(iCurrentEdge);
				visitedFaces.insert(currentEdge.face);

				// 2. Get the next face as the one found by crossing the
				// currentEdge, more specifically, the HEFace of the opposite
				// HEEdge
				HEEdge oppositeEdge = meshData.getEdge(currentEdge.oppositeEdge);
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
					HEVertex nextFaceVertex = meshData.getVertex(oppositeEdge.vertex);
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
