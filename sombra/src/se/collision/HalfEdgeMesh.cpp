#include <iterator>
#include <algorithm>
#include "se/collision/HalfEdgeMesh.h"

#define NORMALIZATION_ZERO 0.0001f

namespace se::collision {

	int addVertex(HalfEdgeMesh& meshData, const glm::vec3& point)
	{
		return meshData.vertices.emplace(point).getIndex();
	}


	void removeVertex(HalfEdgeMesh& meshData, int iVertex)
	{
		if (!meshData.edges.isActive(iVertex)) { return; }

		int iInitialEdge = meshData.vertices[iVertex].edge;
		int iCurrentEdge = iInitialEdge;
		do {
			if (meshData.edges.isActive(iCurrentEdge)) {
				const HEEdge& currentEdge = meshData.edges[iCurrentEdge];
				const HEEdge& oppositeEdge = meshData.edges[currentEdge.oppositeEdge];

				if (meshData.faces.isActive(currentEdge.face)) {
					removeFace(meshData, currentEdge.face);
				}

				iCurrentEdge = oppositeEdge.nextEdge;
			}
			else {
				iCurrentEdge = iInitialEdge;
			}
		}
		while (iCurrentEdge != iInitialEdge);

		meshData.vertices.erase( meshData.vertices.begin().setIndex(iVertex) );
	}


	int addEdge(HalfEdgeMesh& meshData, int iVertex1, int iVertex2)
	{
		int iEdge = -1;

		if (meshData.vertices.isActive(iVertex1) && meshData.vertices.isActive(iVertex2)
			&& (iVertex1 != iVertex2)
			&& (meshData.vertexEdgeMap.find(std::make_pair(iVertex1, iVertex2)) == meshData.vertexEdgeMap.end())
		) {
			// Create the HEEdge and its opposite one
			int iEdge1 = meshData.edges.emplace().getIndex();
			int iEdge2 = meshData.edges.emplace().getIndex();

			// Set the HEVertices of both HEEdges
			meshData.edges[iEdge1].vertex = iVertex2;
			meshData.edges[iEdge2].vertex = iVertex1;

			// Set the opposite HEEdge of both HEEdges
			meshData.edges[iEdge1].oppositeEdge = iEdge2;
			meshData.edges[iEdge2].oppositeEdge = iEdge1;

			// Register the HEEdges on the map
			meshData.vertexEdgeMap[std::make_pair(iVertex1, iVertex2)] = iEdge1;
			meshData.vertexEdgeMap[std::make_pair(iVertex2, iVertex1)] = iEdge2;

			// Set the HEEdge of the HEVertices
			if (meshData.vertices[iVertex1].edge < 0) {
				meshData.vertices[iVertex1].edge = iEdge1;
			}
			if (meshData.vertices[iVertex2].edge < 0) {
				meshData.vertices[iVertex2].edge = iEdge2;
			}

			iEdge = iEdge1;
		}

		return iEdge;
	}


	void removeEdge(HalfEdgeMesh& meshData, int iEdge)
	{
		if (!meshData.edges.isActive(iEdge)) { return; }

		HEEdge& currentEdge = meshData.edges[iEdge];
		int iOppositeEdge = currentEdge.oppositeEdge;
		HEEdge& oppositeEdge = meshData.edges[iOppositeEdge];

		// Remove the currentEdge and its opposite one only if the opposite
		// HEEdge hasn't a HEFace
		if (oppositeEdge.face < 0) {
			int iVertex1 = oppositeEdge.vertex;
			int iVertex2 = currentEdge.vertex;

			// Remove the HEEdges from the map
			meshData.vertexEdgeMap.erase(std::make_pair(iVertex1, iVertex2));
			meshData.vertexEdgeMap.erase(std::make_pair(iVertex2, iVertex1));

			// Remove the HEEdges
			meshData.edges.erase( meshData.edges.begin().setIndex(iOppositeEdge) );
			meshData.edges.erase( meshData.edges.begin().setIndex(iEdge) );

			// Update the HEEdge of the HEVertices
			if (meshData.vertices[iVertex1].edge == iEdge) {
				meshData.vertices[iVertex1].edge = -1;
				for (const auto& pair : meshData.vertexEdgeMap) {
					if (pair.first.first == iVertex1) {
						meshData.vertices[iVertex1].edge = pair.second;
						break;
					}
				}
			}
			if (meshData.vertices[iVertex2].edge == iOppositeEdge) {
				meshData.vertices[iVertex2].edge = -1;
				for (const auto& pair : meshData.vertexEdgeMap) {
					if (pair.first.first == iVertex2) {
						meshData.vertices[iVertex2].edge = pair.second;
						break;
					}
				}
			}
		}
		else {
			// Reset the HEFace data of the current HEEdge
			currentEdge.face			= -1;
			currentEdge.previousEdge	= -1;
			currentEdge.nextEdge		= -1;
		}
	}


	void removeFace(HalfEdgeMesh& meshData, int iFace)
	{
		if (!meshData.faces.isActive(iFace)) { return; }

		int iInitialEdge = meshData.faces[iFace].edge;
		int iCurrentEdge = iInitialEdge;
		do {
			HEEdge& currentEdge = meshData.edges[iCurrentEdge];
			int iNextEdge = currentEdge.nextEdge;

			removeEdge(meshData, iCurrentEdge);

			iCurrentEdge = iNextEdge;
		}
		while (iCurrentEdge != iInitialEdge);

		meshData.faces.erase( meshData.faces.begin().setIndex(iFace) );
	}


	int mergeFaces(HalfEdgeMesh& meshData, int iFace1, int iFace2)
	{
		struct Section { int iInitialEdge, iFinalEdge, length; };

		if (!meshData.faces.isActive(iFace1) || !meshData.faces.isActive(iFace2)) { return -1; }
		if (iFace1 == iFace2) { return iFace1; }

		// Find the HEEdge loop sections shared between both HEFaces
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

		if (sections.empty()) { return -1; }

		// Find the longest shared HEEdge loop section between the HEFaces
		const Section& section = *std::max_element(
			sections.begin(), sections.end(),
			[](const Section& s1, const Section& s2) { return s1.length > s2.length; }
		);

		// Close the new HEEdge loop of the first HEFace
		const HEEdge& initialEdge = meshData.edges[section.iInitialEdge];
		const HEEdge& oppositeInitialEdge = meshData.edges[initialEdge.oppositeEdge];
		meshData.edges[initialEdge.previousEdge].nextEdge = oppositeInitialEdge.nextEdge;
		meshData.edges[oppositeInitialEdge.nextEdge].previousEdge = initialEdge.previousEdge;

		const HEEdge& finalEdge = meshData.edges[section.iFinalEdge];
		const HEEdge& oppositeFinalEdge = meshData.edges[finalEdge.oppositeEdge];
		meshData.edges[finalEdge.nextEdge].previousEdge = oppositeFinalEdge.previousEdge;
		meshData.edges[oppositeFinalEdge.previousEdge].nextEdge = finalEdge.nextEdge;

		// Update HEEdge of the first HEFace
		meshData.faces[iFace1].edge = initialEdge.previousEdge;

		// Set the HEEdges' HEFace to the first one
		iInitialEdge = meshData.faces[iFace1].edge;
		iCurrentEdge = iInitialEdge;
		do {
			HEEdge& currentEdge = meshData.edges[iCurrentEdge];
			currentEdge.face = iFace1;
			iCurrentEdge = currentEdge.nextEdge;
		}
		while (iCurrentEdge != iInitialEdge);

		// Remove the shared HEEdge section
		iCurrentEdge = section.iInitialEdge;
		int iFinalEdge = finalEdge.nextEdge;
		while (iCurrentEdge != iFinalEdge) {
			const HEEdge& currentEdge = meshData.edges[iCurrentEdge];
			int iNextEdge = currentEdge.nextEdge;

			removeEdge(meshData, currentEdge.oppositeEdge);
			removeEdge(meshData, iCurrentEdge);

			iCurrentEdge = iNextEdge;
		}

		// Erase the second HEFace
		meshData.faces.erase( meshData.faces.begin().setIndex(iFace2) );

		return iFace1;
	}

#ifndef NDEBUG
	std::vector<int> getFaceIndices(const HalfEdgeMesh& meshData, int iFace)
	{
		std::vector<int> result;
		getFaceIndices(meshData, iFace, std::back_inserter(result));
		return result;
	}
#endif

}
