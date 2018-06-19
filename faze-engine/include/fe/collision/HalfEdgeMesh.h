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

		/** @return	the total number of HEVertices in the Mesh */
		int getNumVertices() const { return mVertices.size(); };

		/** Returns the requested HEVertex
		 *
		 * @param	i the HEVertex index
		 * @return	a const reference to the HEVertex */
		const HEVertex& getVertex(int i) const { return mVertices[i]; };

		/** Adds the given point as a new HEVertex in the Mesh
		 *
		 * @param	point the 3D coordintes of the new HEVertex
		 * @return	the index of the HEVertex */
		int addVertex(const glm::vec3& point);

		/** Removes the given HEVertex and the HEFaces and HEEdges that
		 * references it
		 *
		 * @param	i the index of the HEVertex to remove */
		void removeVertex(int i);

		/** @return	a reference to the HEEdges vector of the Mesh */
		CachedVector<HEEdge>& getEdgesVector() { return mEdges; };

		/** @return	a const reference to the HEEdges vector of the Mesh */
		const CachedVector<HEEdge>& getEdgesVector() const { return mEdges; };

		/** @return	the total number of HEEdges in the Mesh */
		int getNumEdges() const { return mEdges.size(); };

		/** Returns the requested HEEdge
		 *
		 * @param	i the HEEdge index
		 * @return	a const reference to the HEEdge */
		const HEEdge& getEdge(int i) const { return mEdges[i]; };

		/** @return	a reference to the HEFaces vector of the Mesh */
		CachedVector<HEFace>& getFacesVector() { return mFaces; };

		/** @return	a const reference to the HEFaces vector of the Mesh */
		const CachedVector<HEFace>& getFacesVector() const { return mFaces; };

		/** @return	the total number of HEFaces in the Mesh */
		int getNumFaces() const { return mFaces.size(); };

		/** Returns the requested HEFace
		 *
		 * @param	i the HEFace index
		 * @return	a const reference to the HEFace */
		const HEFace& getFace(int i) const { return mFaces[i]; };

		/** Creates a new HEFace from the given HEVertex indices and adds it
		 * to the Mesh
		 *
		 * @param	vertexIndices the indices of the HEVertex of the new HEFace
		 * @return	the index of the HEFace */
		int addFace(const std::vector<int>& vertexIndices);

		/** Removes the given HEFace and the HEEdges that references it
		 *
		 * @param	i the index of the HEFace to remove */
		void removeFace(int i);

		/** Merges two HEFaces into a single one by the given HEEdge
		 *
		 * @param	iEdge the shared HEEdge between the HEFaces to merge */
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


	/** Calculates the boundary of the convex hull as seen from the given
	 * eye point
	 *
	 * @param	eyePoint the 3D coordinates of the eye point
	 * @param	iFace the index of the initial HEFace from which we will start
	 *			searching
	 * @param	meshData the Mesh data with the HEVertices and HEFaces
	 * @return	a pair with the list of HEEdge indices that represents the
	 *			boundary of the ConvexHull and the list of HEFace indices with
	 *			the visible HEFaces
	 * @note	the initial HEFace must be visible from the eyePoint
	 *			perspective */
	std::pair<std::vector<int>, std::vector<int>> calculateHorizon(
		const glm::vec3& eyePoint,
		int iFace, const HalfEdgeMesh& meshData
	);

}}

#endif		// HALF_EDGE_MESH_H
