#ifndef QUICK_HULL_H
#define QUICK_HULL_H

#include <map>
#include <vector>
#include <glm/glm.hpp>
#include "fe/collision/HalfEdgeMesh.h"

namespace fe { namespace collision {

	/**
	 * Class QuickHull, it's used to calculate the 3D convex hull of any given
	 * 3D Half-Edge Mesh
	 */
	class QuickHull
	{
	private:	// Attributes
		/** The Half-Edge Mesh with the convex hull of the current Mesh */
		HalfEdgeMesh mConvexHull;

		/** Maps each convex hull face with its normal vector */
		std::map<int, glm::vec3> mFaceNormals;

		/** Maps each convex hull face with its outside vertex indices */
		std::map<int, std::vector<int>> mFaceOutsideVertices;

		/** Maps the indices of the vertices in the current Mesh with the ones
		 * in the convex hull */
		std::map<int, int> mVertexIndexMap;

	public:		// Functions
		/** Creates a new QuickHull object */
		QuickHull() {};

		/** Class destructor */
		~QuickHull() {};

		/** Calculates the convex hull of the given 3D Mesh with the QuickHull
		 * 3D algorithm
		 *
		 * @param	meshData the Half-Edge data structure with the 3D Mesh to
		 *			calculate its convex hull
		 * @return	the convex hull of the given Mesh */
		HalfEdgeMesh calculate(const HalfEdgeMesh& meshData);
	private:
		/** Calculates the initial simplex needed for calculating the QuickHull
		 * algorithm from the Mesh vertices and stores it in mConvexHull
		 *
		 * @param	meshData the Mesh with the vertices vertices with which we
		 *			will calculate the initial simplex */
		void createInitialConvexHull(const HalfEdgeMesh& meshData);

		/** Calculates the indices of the vertices that creates an initial
		 * simplex
		 *
		 * @param	meshData the Mesh with the vertices vertices with which we
		 *			will calculate the initial simplex
		 * @return	the indices of vertices of the initial simplex */
		std::vector<int> calculateInitialSimplex(
			const HalfEdgeMesh& meshData
		) const;

		/** Calculates which of the given vertices are outside of the current
		 * Hull by the given HEFace
		 *
		 * @param	vertexIndices the index to the vertices of the mesh that we
		 *			want to test
		 * @param	meshData the Mesh data with the Vertices
		 * @param	iFace the index of the HEFace in the convex hull
		 * @return	the indices of the vertices that are in front of the face */
		std::vector<int> getVerticesOutside(
			const std::vector<int>& vertexIndices, const HalfEdgeMesh& meshData,
			int iFace
		) const;

		/** Calculates which of the given Vertices is the the furthest point in
		 * the HEFace normal direction
		 *
		 * @param	vertexIndices the index of the meshData Vertices to check
		 * @param	iFace the index of the HEFace of the ConvexHull mesh
		 * @param	meshData the Mesh data that contains the Vertices
		 * @return	the index of the furthest meshData HEVertex, -1 if iFace is
		 *			not valid or vertexIndices is empty */
		int getFurthestVertex(
			const std::vector<int>& vertexIndices,
			int iFace, const HalfEdgeMesh& meshData
		) const;

		/** Merges the given HEFace of the ConvexHull with its surrounding
		 * HEFaces if they are coplanar (they have the same normal vector)
		 *
		 * @param	iFace the index of the HEFace of the ConvexHull to merge */
		void mergeCoplanarFaces(int iFace);
	};

}}

#endif		// QUICK_HULL_H
