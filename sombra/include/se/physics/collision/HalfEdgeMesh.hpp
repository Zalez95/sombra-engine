#ifndef HALF_EDGE_MESH_HPP
#define HALF_EDGE_MESH_HPP

namespace se::physics {

	template <class InputIterator>
	int addFace(HalfEdgeMesh& meshData, const InputIterator& begin, const InputIterator& end)
	{
		if (std::distance(begin, end) < 3) { return -1; }

		// Create a new HEFace
		int iFace = static_cast<int>(meshData.faces.emplace().getIndex());

		// Set the HEFace and HEVertices data and recover all the HEEdges
		bool fail = false;
		std::vector<int> edgeIndices;
		for (auto it = begin; (it != end) && !fail; ++it) {
			int iVertex1 = *it,
				iVertex2 = (it + 1 == end)? *begin : *(it + 1);

			// Get the HEEdge of the current vertices
			auto edgeIt = meshData.vertexEdgeMap.find( std::make_pair(iVertex1, iVertex2) );
			int iEdge = (edgeIt != meshData.vertexEdgeMap.end())? edgeIt->second : addEdge(meshData, iVertex1, iVertex2);
			if (iEdge >= 0) {
				edgeIndices.push_back(iEdge);

				// Set the HEEdge of the HEFace
				if (meshData.faces[iFace].edge < 0) {
					meshData.faces[iFace].edge = iEdge;
				}
			}
			else {
				fail = true;
			}
		}

		// Set the previous and next HEEdges of the HEFace's HEEdges
		for (auto it = edgeIndices.begin(); (it != edgeIndices.end()) && !fail; ++it) {
			HEEdge& currentEdge = meshData.edges[*it];
			if (currentEdge.face < 0) {
				currentEdge.face			= iFace;
				currentEdge.previousEdge	= (it == edgeIndices.begin())? edgeIndices.back() : *(it - 1);
				currentEdge.nextEdge		= (it + 1 == edgeIndices.end())? edgeIndices.front() : *(it + 1);
			}
			else {
				fail = true;
			}
		}

		// Clean the HEFace data if an HEEdge was already in use in another one
		if (fail) {
			for (int iEdge : edgeIndices) { removeEdge(meshData, iEdge); }
			meshData.faces.erase( meshData.faces.begin().setIndex(iFace) );
			iFace = -1;
		}

		return iFace;
	}


	template <class OutputIterator>
	void getFaceIndices(const HalfEdgeMesh& meshData, int iFace, OutputIterator result)
	{
		if (!meshData.faces.isActive(iFace)) { return; }

		int iInitialEdge = meshData.faces[iFace].edge;
		int iCurrentEdge = iInitialEdge;
		do {
			const HEEdge& currentEdge	= meshData.edges[iCurrentEdge];
			const HEEdge& oppositeEdge	= meshData.edges[currentEdge.oppositeEdge];
			*(result++) = oppositeEdge.vertex;
			iCurrentEdge = currentEdge.nextEdge;
		}
		while ((iCurrentEdge != iInitialEdge) && meshData.edges.isActive(iCurrentEdge));
	}

}

#endif		// HALF_EDGE_MESH_HPP
