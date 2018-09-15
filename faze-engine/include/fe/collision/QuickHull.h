#ifndef QUICK_HULL_H
#define QUICK_HULL_H

#include <map>
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
		/** The precision with which we will compare the HEVertices to the
		 * HEFaces of the Meshes when checking if they're over or inside
		 * them */
		const float mEpsilon;

		/** The Half-Edge Mesh with the convex hull of the current Mesh */
		HalfEdgeMesh mConvexHull;

		/** Maps each convex hull face with its normal vector */
		std::map<int, glm::vec3> mFaceNormals;

		/** Maps each convex hull face with its outside vertex indices.
		 * @note	the outside vertex indices are sorted ascendently */
		std::map<int, std::vector<int>> mFaceOutsideVertices;

		/** Maps the indices of the vertices in the current Mesh with the ones
		 * in the convex hull */
		std::map<int, int> mVertexIndexMap;

	public:		// Functions
		/** Creates a new QuickHull object
		 *
		 * @param	epsilon the epsilon value of the comparisons during the
		 *			QuickHull algorithm computation */
		QuickHull(float epsilon) : mEpsilon(epsilon) {};

		/** Class destructor */
		~QuickHull() {};

		/** @return	the HalfEdgeMesh of the convex hull */
		const HalfEdgeMesh& getMesh() const { return mConvexHull; };

		/** @return	the map with the normal vectors of the HEFaces of the
		 *			convex hull HalfEdgeMesh */
		const std::map<int, glm::vec3>& getNormalsMap() const
		{ return mFaceNormals; };

		/** Calculates the convex hull of the given 3D Mesh with the QuickHull
		 * 3D algorithm
		 *
		 * @param	meshData the Half-Edge data structure with the 3D Mesh to
		 *			calculate its convex hull */
		void calculate(const HalfEdgeMesh& meshData);

		/** Resets the convex hull data for the next calculations */
		void resetData();
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
		 * @return	the indices of the vertices that are in front of the face.
		 *			The order of the vertices will be preserved */
		std::vector<int> getVerticesOutside(
			const std::vector<int>& vertexIndices, const HalfEdgeMesh& meshData,
			int iFace
		) const;

		/** Calculates which of the given HEVertices is the the furthest point
		 * in the HEFace normal direction
		 *
		 * @param	vertexIndices the index of the meshData Vertices to check
		 * @param	meshData the Mesh data that contains the Vertices
		 * @param	direction the direction towards we want to find the furthest
		 *			HEVertex
		 * @return	the index of the furthest meshData HEVertex, -1 if iFace is
		 *			not valid or vertexIndices is empty */
		int getFurthestVertexFrom(
			const std::vector<int>& vertexIndices, const HalfEdgeMesh& meshData,
			const glm::vec3& direction
		) const;

		/** Merges the given HEFace of the ConvexHull with its surrounding
		 * HEFaces if they are coplanar (they have the same normal vector)
		 *
		 * @param	iFace the index of the HEFace of the ConvexHull to merge */
		void mergeCoplanarFaces(int iFace);
	};

}}

#endif		// QUICK_HULL_H
