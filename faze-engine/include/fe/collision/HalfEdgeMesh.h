#ifndef HALF_EDGE_MESH_H
#define HALF_EDGE_MESH_H

#include <map>
#include <glm/glm.hpp>
#include "ContiguousVector.h"

namespace fe { namespace collision {

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
		~HEVertex() {};
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
		~HEEdge() {};
	};


	/**
	 * Struct HEFace
	 */
	struct HEFace
	{
		/** The index of one of the HEEdges of the HEFace */
		int edge;

		HEFace() : edge(-1) {};
		~HEFace() {};
	};


	/**
	 * Struct HalfEdgeMesh, it holds the data of a 3D Mesh in a Half-Edge
	 * data structure so we can store the adjacency of the Faces and Edges
	 * for faster computations.
	 *
	 * @note	a Half-Edge mesh is only capable of representing manifold
	 *			surfaces, this means that every HEEdge is bordered by only
	 *			two HEFaces.
	 */
	struct HalfEdgeMesh
	{
		/** The HEVertices of the Mesh */
		ContiguousVector<HEVertex> vertices;

		/** The HEEdges of the Mesh */
		ContiguousVector<HEEdge> edges;

		/** The HEFaces of the Mesh */
		ContiguousVector<HEFace> faces;

		/** Maps two HEVertex indices with the HEEdge that references them */
		std::map<std::pair<int, int>, int> vertexEdgeMap;
	};


	/** Adds the given point as a new HEVertex in the Mesh
	 *
	 * @param	meshData the Mesh to add the HEVertex
	 * @param	point the 3D coordintes of the new HEVertex
	 * @return	the index of the new HEVertex */
	int addVertex(HalfEdgeMesh& meshData, const glm::vec3& point);


	/** Removes the given HEVertex and the HEFaces and HEEdges that
	 * references it from the given Mesh
	 *
	 * @param	meshData the Mesh where the HEVertex is located in
	 * @param	iVertex the index of the HEVertex to remove */
	void removeVertex(HalfEdgeMesh& meshData, int iVertex);


	/** Creates a new HEFace from the given HEVertex indices and adds it
	 * to the Mesh
	 *
	 * @param	meshData the Mesh to add the HEFace
	 * @param	vertexIndices the indices of the HEVertex of the new HEFace
	 * @return	the index of the new HEFace, -1 if the number of HEVertices is
	 *			less than 3 */
	int addFace(HalfEdgeMesh& meshData, const std::vector<int>& vertexIndices);


	/** Removes the given HEFace and the HEEdges that references it from the
	 * given Mesh
	 *
	 * @param	meshData the Mesh where the HEFace is located in
	 * @param	iFace the index of the HEFace to remove
	 * @note	the HEEdges of the HEEFace will only be removed if its
	 *			opposite one hasn't setted its HEFace, otherwise it will
	 *			only update its internal data (remove the loop data and the
	 *			face) */
	void removeFace(HalfEdgeMesh& meshData, int iFace);


	/** Merges the given two HEFaces into a single one
	 *
	 * @param	meshData the Mesh where the HEFaces are located in
	 * @param	iFace1 the first of the HEFaces to merge. This HEFace will
	 *			be the one preserved
	 * @param	iFace2 the second of the HEFaces to merge. This HEFace will
	 *			be the one removed
	 * @return	the index of the merged HEFace, -1 if the HEFaces couldn't be
	 *			merged */
	int mergeFaces(HalfEdgeMesh& meshData, int iFace1, int iFace2);


	/** Replaces the polygon HEFaces of the given HalfEdgeMesh for triangles
	 *
	 * @param	meshData the Mesh where the HEFaces are located in
	 * @note	we will use the ear clipping method, which only works with
	 *			convex polygons (The HEFaces are ensured to be convex if we
	 *			had added them with the addFace method) */
	void triangulateFaces(HalfEdgeMesh& meshData);


	/** Returns the HEVertex indices of the given HEFace
	 *
	 * @param	meshData the Mesh where the HEFace is located in
	 * @param	iFace the index of the HEFace
	 * @return	a vector with the indices of the HEFace's HEVertices */
	std::vector<int> getFaceIndices(const HalfEdgeMesh& meshData, int iFace);


	/** Calculates the normal vector of the mesh surface at the given HEVertex
	 *
	 * @param	meshData the mesh that holds the HEVertex
	 * @param	faceNormals a map with the normals of each HEFace of the mesh
	 * @param	iVertex the index of the HEVertex where we want to calculate
	 *			the normal vector
	 * @return	the normal vector */
	glm::vec3 calculateVertexNormal(
		const HalfEdgeMesh& meshData,
		const std::map<int, glm::vec3>& faceNormals,
		int iVertex
	);


	/** Calculates the normal of the given HEFace
	 *
	 * @param	meshData the Mesh where the HEFace is located in
	 * @param	iFace the index of the HEFace
	 * @return	the normal of the HEFace */
	glm::vec3 calculateFaceNormal(const HalfEdgeMesh& meshData, int iFace);


	/** Calculates the furthest point of the Mesh in the given direction with
	 * the Hill-Climbing algorithm
	 *
	 * @param	meshData the Mesh that holds the HEVertices and HEFaces
	 * @param	direction the direction in which we are going to search
	 * @return	the index of the furthest Mesh HEVertex
	 * @note	the Mesh must be convex, otherwise the furthest point found
	 *			could be a local maximum */
	int getFurthestVertexInDirection(
		const HalfEdgeMesh& meshData,
		const glm::vec3& direction
	);


	/** Calculates the boundary of the given HalfEdgeMesh as seen from the given
	 * eye point
	 *
	 * @param	meshData the Mesh that holds the HEVertices and HEFaces
	 * @param	faceNormals the normals of each HEFace
	 * @param	eyePoint the 3D coordinates of the eye point
	 * @param	iFace the index of the initial HEFace from which we will start
	 *			searching
	 * @return	a pair with the list of HEEdge indices that represents the
	 *			boundary of the ConvexHull and the list of HEFace indices with
	 *			the visible HEFaces
	 * @note	the initial HEFace must be visible from the eyePoint
	 *			perspective */
	std::pair<std::vector<int>, std::vector<int>> calculateHorizon(
		const HalfEdgeMesh& meshData,
		const std::map<int, glm::vec3>& faceNormals,
		const glm::vec3& eyePoint, int iFace
	);

}}

#endif		// HALF_EDGE_MESH_H
