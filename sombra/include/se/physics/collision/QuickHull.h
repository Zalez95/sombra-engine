#ifndef QUICK_HULL_H
#define QUICK_HULL_H

#include <glm/glm.hpp>
#include "HalfEdgeMesh.h"

namespace se::physics {

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

		/** The precision of the comparison scaled with the size of the
		 * HalfEdgeMesh to calculate */
		float mScaledEpsilon;

		/** The Half-Edge Mesh with the convex hull of the current Mesh */
		HalfEdgeMesh mConvexHullMesh;

		/** The normal vector of each HEFace in the convex hull */
		utils::PackedVector<glm::vec3> mConvexHullNormals;

		/** The outside vertex indices of each HEFace in the convex hull
		 * @note	the outside vertex indices are sorted ascendently */
		utils::PackedVector<std::vector<int>> mFaceOutsideVertices;

		/** Maps the indices of the vertices in the current Mesh with the ones
		 * in the convex hull */
		std::unordered_map<int, int> mVertexIndexMap;

	public:		// Functions
		/** Creates a new QuickHull object
		 *
		 * @param	epsilon the epsilon value of the comparisons during the
		 *			QuickHull algorithm computation */
		QuickHull(float epsilon) :
			mEpsilon(epsilon), mScaledEpsilon(epsilon) {};

		/** @return	the HalfEdgeMesh of the convex hull */
		const HalfEdgeMesh& getMesh() const { return mConvexHullMesh; };

		/** @return	the normal vectors of the HEFaces of the convex hull
		 *			HalfEdgeMesh */
		const utils::PackedVector<glm::vec3>& getNormals() const
		{ return mConvexHullNormals; };

		/** Calculates the convex hull of the given Mesh with the QuickHull
		 * algorithm
		 *
		 * @param	originalMesh the Half-Edge data structure with the Mesh to
		 *			calculate its convex hull */
		void calculate(const HalfEdgeMesh& originalMesh);

		/** Resets the convex hull data for the next calculations */
		void resetData();
	private:
		/** Calculates the indices of the vertices that creates an initial
		 * simplex
		 *
		 * @param	meshData the Mesh with the vertices vertices with which we
		 *			will calculate the initial simplex
		 * @return	the indices of vertices of the initial simplex */
		std::vector<int> calculateInitialSimplex(
			const HalfEdgeMesh& meshData
		) const;

		/** Calculates the convex hull of the given 2D Mesh with the QuickHull
		 * 2D algorithm
		 *
		 * @param	originalMesh the Half-Edge data structure with the 2D Mesh
		 *			to calculate its convex hull
		 * @param	iSimplexVertices the indices of the simplex vertices
		 * @note	the mesh must have at least 3 vertices */
		void calculateQuickHull2D(
			const HalfEdgeMesh& originalMesh,
			const std::vector<int>& iSimplexVertices
		);

		/** Calculates which of the given vertices is the furthest from the
		 * given edge
		 *
		 * @param	vertices the 3D positions of the vertices
		 * @param	vertexIndices the indices of the vertices to evaluate
		 * @param	iVertex1 the first of the vertices of the edge
		 * @param	iVertex2 the second of the vertices of the edge
		 * @return	the index of the furthest vertex */
		int getFurthestVertexFromEdge(
			const utils::PackedVector<HEVertex>& vertices,
			const std::vector<int>& vertexIndices, int iVertex1, int iVertex2
		) const;

		/** Checks which of the given vertices are outside of the edge between
		 * the vertex 1 and 2
		 *
		 * @param	vertices the coordinates of the vertices to check
		 * @param	vertexIndices the indices of the vertices to check
		 * @param	planeNormal the normal vector of the plane where all the
		 *			vertices lie on
		 * @param	iVertex1 the index of the first vertex of the edge
		 * @param	iVertex1 the index of the second vertex of the edge
		 * @return	the indices of the vertices that are outside the edge */
		std::vector<int> filterOutsideVertices(
			const utils::PackedVector<HEVertex>& vertices,
			const std::vector<int>& vertexIndices,
			const glm::vec3& planeNormal, int iVertex1, int iVertex2
		) const;

		/** Calculates the convex hull of the given 3D Mesh with the QuickHull
		 * 3D algorithm
		 *
		 * @param	originalMesh the Half-Edge data structure with the 3D Mesh
		 *			to calculate its convex hull
		 * @param	iSimplexVertices the indices of the simplex vertices */
		void calculateQuickHull3D(
			const HalfEdgeMesh& originalMesh,
			const std::vector<int>& iSimplexVertices
		);

		/** Calculates the initial HEMesh needed for calculating the 3D
		 * QuickHull algorithm from the given Mesh vertices
		 *
		 * @param	originalMesh the Half-Edge data structure with the 3D Mesh
		 *			to calculate its initial convex hull
		 * @param	iSimplexVertices the indices of the simplex vertices */
		void createInitial3DConvexHull(
			const HalfEdgeMesh& originalMesh,
			const std::vector<int>& iSimplexVertices
		);

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

		/** Calculates which of the given HEVertices is the the furthest in
		 * the given direction
		 *
		 * @param	vertexIndices the index of the meshData Vertices to check
		 * @param	meshData the Mesh data that contains the Vertices
		 * @param	direction the direction towards we want to find the furthest
		 *			HEVertex
		 * @return	the index of the furthest meshData HEVertex, -1 if iFace is
		 *			not valid or vertexIndices is empty */
		int getFurthestVertexInDirection(
			const std::vector<int>& vertexIndices, const HalfEdgeMesh& meshData,
			const glm::vec3& direction
		) const;

		/** Merges the given HEFace of the ConvexHull with its surrounding
		 * HEFaces if they are coplanar (they have the same normal vector)
		 *
		 * @param	iFace the index of the HEFace of the ConvexHull to merge */
		void mergeCoplanarFaces(int iFace);
	};

}

#endif		// QUICK_HULL_H
