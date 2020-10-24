#ifndef HALF_EDGE_MESH_H
#define HALF_EDGE_MESH_H

#include <unordered_map>
#include <glm/glm.hpp>
#include "../utils/PackedVector.h"
#include "../utils/MathUtils.h"

namespace se::collision {

	/**
	 * Struct HEVertex
	 */
	struct HEVertex
	{
		/** The 3D coordinates of the HEVertex */
		glm::vec3 location;

		/** The index of one of the outgoing HEEdges */
		int edge;

		HEVertex() : location(0.0f), edge(-1) {};
		HEVertex(const glm::vec3& location) : location(location), edge(-1) {};
	};


	/**
	 * Struct HEEdge
	 */
	struct HEEdge
	{
		/** The index of the vertex which the HEEdge is pointing to */
		int vertex;

		/** The index of the HEFace of the HEEdge */
		int face;

		/** The index of the previous HEEdge in the same Face */
		int previousEdge;

		/** The index of the next HEEdge in the same Face */
		int nextEdge;

		/** The index of an HEEdge pointing in the opposite direction */
		int oppositeEdge;

		HEEdge() :
			vertex(-1), face(-1),
			previousEdge(-1), nextEdge(-1), oppositeEdge(-1) {};
	};


	/**
	 * Struct HEFace
	 */
	struct HEFace
	{
		/** The index of one of the HEEdges of the HEFace */
		int edge;

		HEFace() : edge(-1) {};
	};


	/**
	 * Struct HalfEdgeMesh, it holds the data of a 3D mesh in a Half-Edge
	 * data structure so we can store the adjacency of the Faces and Edges
	 * for faster computations.
	 *
	 * @note	a Half-Edge mesh is only capable of representing manifold
	 *			surfaces, this means that every HEEdge is bordered by only
	 *			two HEFaces.
	 */
	struct HalfEdgeMesh
	{
		/** The HEVertices of the HalfEdgeMesh */
		utils::PackedVector<HEVertex> vertices;

		/** The HEEdges of the HalfEdgeMesh */
		utils::PackedVector<HEEdge> edges;

		/** The HEFaces of the HalfEdgeMesh */
		utils::PackedVector<HEFace> faces;

		/** Maps two HEVertex indices with the HEEdge that references them */
		std::unordered_map<std::pair<int, int>, int, utils::PairHash>
			vertexEdgeMap;
	};


	/** Adds the given point as a new HEVertex in the HalfEdgeMesh
	 *
	 * @param	meshData the HalfEdgeMesh to add the HEVertex
	 * @param	point the 3D coordintes of the new HEVertex
	 * @return	the index of the new HEVertex in the HalfEdgeMesh vertices
	 *			PackedVector (the HEVertex is added with the emplace
	 *			function) */
	int addVertex(HalfEdgeMesh& meshData, const glm::vec3& point);


	/** Removes the given HEVertex and the HEFaces and HEEdges that
	 * references it from the given HalfEdgeMesh
	 *
	 * @param	meshData the HalfEdgeMesh where the HEVertex is located in
	 * @param	iVertex the index of the HEVertex to remove */
	void removeVertex(HalfEdgeMesh& meshData, int iVertex);


	/** Creates a new HEEdge from the given HEVertex indices and adds it
	 * to the HalfEdgeMesh
	 *
	 * @param	meshData the HalfEdgeMesh to add the HEEdge
	 * @param	iVertex1 the index of first HEVertex of the HalfEdgeMesh
	 * @param	iVertex2 the index of second HEVertex of the HalfEdgeMesh. It's
	 *			the HEVertex towards the new HEEdge will point to
	 * @return	the index of the new HEEdge, -1 if there already is another
	 *			HEEdge with the same indices in the HalfEdgeMesh */
	int addEdge(HalfEdgeMesh& meshData, int iVertex1, int iVertex2);


	/** Removes the given HEEdge from the given HalfEdgeMesh
	 *
	 * @param	meshData the HalfEdgeMesh where the HEFace is located in
	 * @param	iEdge the index of the HEEdge to remove
	 * @note	the HEEdge will only be removed if its opposite one hasn't
	 *			setted its HEFace, otherwise it will only update its data
	 *			(remove the loop data and the HEFace index) */
	void removeEdge(HalfEdgeMesh& meshData, int iEdge);


	/** Creates a new HEFace from the given HEVertex indices and adds it
	 * to the HalfEdgeMesh
	 *
	 * @param	meshData the HalfEdgeMesh to add the HEFace
	 * @param	begin an iterator to the first HEVertex index of the new HEFace
	 * @param	end an iterator to the past-the-end HEVertex index of the new
	 *			HEFace
	 * @return	the index of the new HEFace in the HalfEdgeMesh faces
	 *			PackedVector (the HEFace is added with the emplace function),
	 *			-1 if the number of HEVertices is less than 3 */
	template <class InputIterator>
	int addFace(
		HalfEdgeMesh& meshData,
		const InputIterator& begin, const InputIterator& end
	);


	/** Removes the given HEFace and the HEEdges that references it from the
	 * given HalfEdgeMesh
	 *
	 * @param	meshData the HalfEdgeMesh where the HEFace is located in
	 * @param	iFace the index of the HEFace to remove */
	void removeFace(HalfEdgeMesh& meshData, int iFace);


	/** Merges the given two HEFaces into a single one by their longest shared
	 * HEEdge loop section
	 *
	 * @param	meshData the HalfEdgeMesh where the HEFaces are located in
	 * @param	iFace1 the first of the HEFaces to merge. This HEFace will
	 *			be the one preserved
	 * @param	iFace2 the second of the HEFaces to merge. This HEFace will
	 *			be the one removed
	 * @return	the index of the merged HEFace, -1 if the HEFaces couldn't be
	 *			merged */
	int mergeFaces(HalfEdgeMesh& meshData, int iFace1, int iFace2);


	/** Calculates the HEVertex indices of the given HEFace
	 *
	 * @param	meshData the HalfEdgeMesh where the HEFace is located in
	 * @param	iFace the index of the HEFace
	 * @param	result the output iterator where the indices of the HEFace's
	 *			HEVertices will be stored */
	template <class OutputIterator>
	void getFaceIndices(
		const HalfEdgeMesh& meshData, int iFace,
		OutputIterator result
	);

#ifndef NDEBUG
	/** Calculates the HEVertex indices of the given HEFace
	 *
	 * @param	meshData the HalfEdgeMesh where the HEFace is located in
	 * @param	iFace the index of the HEFace
	 * @return	a vector with the indices of the HEFace's HEVertices
	 * @note	this function is only for debugging purposes */
	std::vector<int> getFaceIndices(const HalfEdgeMesh& meshData, int iFace);
#endif		// NDEBUG

}

#include "HalfEdgeMesh.hpp"

#endif		// HALF_EDGE_MESH_H
