#include <array>
#include "MeshAdjacencyData.h"

namespace fe { namespace collision {

	void MeshAdjacencyData::addVertex(const glm::vec3& point)
	{
		vertices.emplace_back(point);
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
		faces.emplace_back();
		int iFace = faces.size() - 1;

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
				edges.emplace_back();
				int iEdge1 = edges.size() - 1;

				// Create the oposite edge
				edges.emplace_back();
				int iEdge2 = edges.size() - 1;

				// Set the vertices of both edges
				edges[iEdge1].vertex = iVertex2;
				edges[iEdge2].vertex = iVertex1;

				// Set the opposite edge of both edges
				edges[iEdge1].oppositeEdge = iEdge2;
				edges[iEdge2].oppositeEdge = iEdge1;

				// Register the edges on the map
				mVertexEdgeMap[std::make_pair(iVertex2, iVertex2)] = iEdge1;
				mVertexEdgeMap[std::make_pair(iVertex2, iVertex1)] = iEdge2;

				iEdge = iEdge1;
			}
			edgeIndexes.push_back(iEdge);

			// Set the Edge of the Face
			if (faces[iFace].edge < 0) {
				faces[iFace].edge = iEdge;
			}

			// Set the Edge of the first Vertex
			if (vertices[iVertex1].edge < 0) {
				vertices[iVertex1].edge = iEdge;
			}

			// Set the Face of the Edge
			edges[iEdge].face = iFace;
		}

		// Set the previous and next edges of the face edges
		for (size_t iEdge = 0; iEdge < edgeIndexes.size(); ++iEdge) {
			int iPreviousEdge	= (iEdge == 0)? edgeIndexes.back() : edgeIndexes[iEdge - 1];
			int iNextEdge		= (iEdge + 1) % edgeIndexes.size();

			Edge& currentEdge = edges[edgeIndexes[iEdge]];
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
	
}}
