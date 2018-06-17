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

		/** Maps each convex hull face with it outside vertex indices */
		std::map<int, std::vector<int>> mFaceOutsideVertices;

		/** Maps the indexes of the vertices in the current Mesh with the ones
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

		/** Calculates the indexes of the vertices that creates an initial
		 * simplex
		 *
		 * @param	meshData the Mesh with the vertices vertices with which we
		 *			will calculate the initial simplex
		 * @return	the indices of vertices of the initial simplex */
		std::vector<int> calculateInitialSimplex(
			const HalfEdgeMesh& meshData
		) const;

		/** Calculates which of the given vertices are outside of the current
		 * Hull by the given Face
		 *
		 * @param	vertexIndices the index to the vertices of the mesh that we
		 *			want to test
		 * @param	meshData the Mesh data with the Vertices
		 * @param	iFace the index of the Face in the convex hull
		 * @return	the indexes of the vertices that in front of the face */
		std::vector<int> getVerticesOutside(
			const std::vector<int>& vertexIndices, const HalfEdgeMesh& meshData,
			int iFace
		) const;

		/** Calculates which of the given Vertices is the the furthest point in
		 * the Face normal direction
		 *
		 * @param	vertexIndices the index of the Vertices to check
		 * @param	iFace the index of the Face
		 * @param	meshData the Mesh data with the Vertices and Faces
		 * @return	the index of the furthest Vertex */
		int getFurthestVertex(
			const std::vector<int>& vertexIndices,
			int iFace, const HalfEdgeMesh& meshData
		) const;
	};

}}

#endif		// QUICK_HULL_H
