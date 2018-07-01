#ifndef HALF_EDGE_MESH_H
#define HALF_EDGE_MESH_H

#include <map>
#include <glm/glm.hpp>
#include "CachedVector.h"

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
	 * Class HalfEdgeMesh, it holds the data of a 3D Mesh in a Half-Edge
	 * data structure so we can store the adjacency of the Faces and Edges
	 * for faster computations
	 */
	class HalfEdgeMesh
	{
	private:		// Attributes
		/** The HEVertices of the Mesh */
		CachedVector<HEVertex> mVertices;

		/** The HEEdges of the Mesh */
		CachedVector<HEEdge> mEdges;

		/** The HEFaces of the Mesh */
		CachedVector<HEFace> mFaces;

		/** Maps two HEVertex indices with the HEEdge that references them */
		std::map<std::pair<int, int>, int> mVertexEdgeMap;

	public:		// Functions
		/** Creates a new HalfEdgeMesh */
		HalfEdgeMesh() {};

		/** Class destructor */
		~HalfEdgeMesh() {};

		/** @return	a reference to the HEVertices vector of the mesh */
		CachedVector<HEVertex>& getVerticesVector() { return mVertices; };

		/** @return	a const reference to the HEVertices vector of the mesh */
		const CachedVector<HEVertex>& getVerticesVector() const
		{ return mVertices; };

		/** Returns the requested HEVertex
		 *
		 * @param	iVertex the HEVertex index
		 * @return	a const reference to the HEVertex */
		const HEVertex& getVertex(int iVertex) const
		{ return mVertices[iVertex]; };

		/** Adds the given point as a new HEVertex in the Mesh
		 *
		 * @param	point the 3D coordintes of the new HEVertex
		 * @return	the index of the HEVertex */
		int addVertex(const glm::vec3& point);

		/** Removes the given HEVertex and the HEFaces and HEEdges that
		 * references it
		 *
		 * @param	iVertex the index of the HEVertex to remove */
		void removeVertex(int iVertex);

		/** @return	a reference to the HEEdges vector of the Mesh */
		CachedVector<HEEdge>& getEdgesVector() { return mEdges; };

		/** @return	a const reference to the HEEdges vector of the Mesh */
		const CachedVector<HEEdge>& getEdgesVector() const { return mEdges; };

		/** Returns the requested HEEdge
		 *
		 * @param	iEdge the HEEdge index
		 * @return	a const reference to the HEEdge */
		const HEEdge& getEdge(int iEdge) const { return mEdges[iEdge]; };

		/** @return	a reference to the HEFaces vector of the Mesh */
		CachedVector<HEFace>& getFacesVector() { return mFaces; };

		/** @return	a const reference to the HEFaces vector of the Mesh */
		const CachedVector<HEFace>& getFacesVector() const { return mFaces; };

		/** Returns the requested HEFace
		 *
		 * @param	iFace the HEFace index
		 * @return	a const reference to the HEFace */
		const HEFace& getFace(int iFace) const { return mFaces[iFace]; };

		/** Creates a new HEFace from the given HEVertex indices and adds it
		 * to the Mesh
		 *
		 * @param	vertexIndices the indices of the HEVertex of the new HEFace
		 * @return	the index of the HEFace, -1 if the number of vertices is
		 *			less than 3 */
		int addFace(const std::vector<int>& vertexIndices);

		/** Removes the given HEFace and the HEEdges that references it
		 *
		 * @param	iFace the index of the HEFace to remove
		 * @note	the HEEdges of the HEEFace will only be removed if its
		 *			opposite one hasn't setted its HEFace, otherwise it will
		 *			only update its internal data (remove the loop data and the
		 *			face) */
		void removeFace(int iFace);

		/** Merges two HEFaces into a single one by the given HEEdge
		 *
		 * @param	iEdge the shared HEEdge between the HEFaces to merge
		 * @note	the removed HEFace will always be the opposite HEFace,
		 *			while the one of the given HEEdge will be preserved and
		 *			updated */
		void mergeFace(int iEdge);
	};


	/** Calculates the normal of the given HEFace
	 *
	 * @param	iFace the index of the HEFace
	 * @param	meshData the data of the Mesh where the Face is located in
	 * @return	the normal of the HEFace */
	glm::vec3 calculateFaceNormal(int iFace, const HalfEdgeMesh& meshData);


	/** Calculates the furthest point of the Mesh in the given direction with
	 * the Hill-Climbing algorithm
	 *
	 * @param	direction the direction in which we are going to search
	 * @param	meshData the Mesh data with the HEVertices and HEFaces
	 * @return	the index of the furthest Mesh HEVertex
	 * @note	the Mesh must be convex, otherwise the furthest point found
	 *			could be a local maximum */
	int getFurthestVertexInDirection(
		const glm::vec3& direction,
		const HalfEdgeMesh& meshData
	);


	/** Calculates the boundary of the given HalfEdgeMesh as seen from the given
	 * eye point
	 *
	 * @param	eyePoint the 3D coordinates of the eye point
	 * @param	iFace the index of the initial HEFace from which we will start
	 *			searching
	 * @param	meshData the Mesh data with the HEVertices and HEFaces
	 * @param	faceNormals the normals of each HEFace
	 * @return	a pair with the list of HEEdge indices that represents the
	 *			boundary of the ConvexHull and the list of HEFace indices with
	 *			the visible HEFaces
	 * @note	the initial HEFace must be visible from the eyePoint
	 *			perspective */
	std::pair<std::vector<int>, std::vector<int>> calculateHorizon(
		const glm::vec3& eyePoint, int iFace,
		const HalfEdgeMesh& meshData,
		const std::map<int, glm::vec3>& faceNormals
	);

}}

#endif		// HALF_EDGE_MESH_H
