#ifndef HACD_H
#define HACD_H

#include "AABB.h"
#include "Graph.h"
#include "HalfEdgeMesh.h"

namespace fe { namespace collision {

	/**
	 * Class HACD, it's used to calculate the aproximate convex decomposition
	 * of any given 3D Half-Edge Mesh
	 */
	class HACD
	{
	private:	// Nested types
		using DualGraphVertex = GraphVertex< std::vector<int> >;
		using DualGraph = Graph< std::vector<int> >;

	private:	// Attributes
		/** The maximum concavity of the dual graph edges needed for HACD
		 * algorithm */
		const float mMaximumConcavity;

		/** The epsilon value needed for the comparisons during the QuickHull
		 * algorithm computation */
		const float mQuickHullEpsilon;

		/** The mesh to apply the HACD algorithm. Its faces must be triangles */
		HalfEdgeMesh mMesh;

		/** The normal vectors of the HEFaces of the Mesh to apply the HACD
		 * algorithm */
		std::map<int, glm::vec3> mFaceNormals;

		/** The Dual Graph asociated with mesh to decompose. Each vertex in
		 * this graph is a triangle in the mesh, and each vertex in the Graph
		 * is connected to another one if the triangle that corresponds to that
		 * vertex shares an edge with another triangle. The data of each vertex
		 * is a vector of ancestor vertices ordered ascendently */
		DualGraph mDualGraph;

		/** The normalization factor used for calculating the decimation cost */
		float mNormalizationFactor;

		/** The aspect ratio factor used for calculating the decimation cost */
		float mAspectRatioFactor;

		/** The convex surfaces in which the concave mesh has been decomposed */
		std::vector<HalfEdgeMesh> mConvexSurfaces;

	public:		// Functions
		/** Creates a new HACD object
		 *
		 * @param	maximumConcavity the maximum concavity needed for HACD
		 *			algorithm
		 * @param	quickHullEpsilon the epsilon value of the comparisons during
		 *			the QuickHull algorithm computation */
		HACD(float maximumConcavity, float quickHullEpsilon) :
			mMaximumConcavity(maximumConcavity),
			mQuickHullEpsilon(quickHullEpsilon) {};

		/** Class destructor */
		~HACD() {};

		/** @return	the HalfEdgeMeshes of the HACD */
		const std::vector<HalfEdgeMesh>& getMeshes() const
		{ return mConvexSurfaces; };

		/** Decomposes the given mesh into multiple convex ones with the HACD
		 * algorithm
		 *
		 * @param	meshData the Half-Edge data structure with the 3D Mesh to
		 *			decompose with the HACD algorithm */
		void calculate(const HalfEdgeMesh& meshData);

		/** Resets the HACD data for the next calculations */
		void resetData();
	private:
		/** Initializes all the data needed for the HACD algorithm computation
		 *
		 * @param	meshData the Half-Edge data structure with the 3D Mesh to
		 *			decompose with the HACD algorithm */
		void initData(const HalfEdgeMesh& meshData);

		/** Creates the initial Dual Graph with the triangles of the Mesh.
 		 *
		 * @param	meshData the Half-Edge Mesh from which we want to create
		 *			the dual graph
		 * @return	the dual graph of the mesh */
		DualGraph createDualGraph(const HalfEdgeMesh& meshData) const;

		/** Calculates the HEFace indices of the surface created from the given
		 * Graph vertices and their ancestors
		 *
		 * @param	iVertex1 the first Graph vertex
		 * @param	iVertex2 the second Graph vertex
		 * @return	the HEFace indices of the surface */
		std::vector<int> calculateSurfaceFaceIndices(
			const DualGraphVertex& vertex1, const DualGraphVertex& vertex2
		) const;

		/** Calculates the maximum concavity of the surface created from the
		 * given the given HAFaces as the maximum distance from a point on the
		 * surface to its convex hull.
		 *
		 * @param	iFaces the indices of the HEFaces of the surface
		 * @param	meshData the HalfEdgeMesh that holds both HEFaces
		 * @param	faceNormals a map with the normal vectors of the meshData
		 *			HEFaces
		 * @return	the concavity of the surface */
		float calculateConcavity(
			const std::vector<int>& iFaces,
			const HalfEdgeMesh& meshData,
			const std::map<int, glm::vec3>& faceNormals
		) const;

		/** Calculate the aspect ratio of the surface resulting of the merge of
		 * the given HEFaces
		 *
		 * @param	iFaces the indices of the HEFaces
		 * @param	meshData the mesh where the HEFaces are located in
		 * @return	the aspect ratio of the surface */
		float calculateAspectRatio(
			const std::vector<int>& iFaces, const HalfEdgeMesh& meshData
		) const;

		/** Calculates the intersection of the given ray on the given mesh
		 *
		 * @param	meshData the convex Mesh to project the point on. The
		 *			points of its faces must lie in the same plane
		 * @param	faceNormals a map with the normals of each HEFace of the
		 *			mesh
		 * @param	origin the origin of the ray. It must be inside of the mesh
		 * @param	direction the direction of the ray
		 * @return	a pair with a flag that tells if the ray intersects the
		 *			mesh and the 3D coordinates of the intersection point */
		std::pair<bool, glm::vec3> raycastInsideMesh(
			const HalfEdgeMesh& meshData,
			const std::map<int, glm::vec3>& faceNormals,
			const glm::vec3& origin, const glm::vec3& direction
		) const;

		/** Calculates the normalization factor as the length of the diagonal
		 * of the MeshCollider's AABB
		 *
		 * @param	aabb the AABB with which we want to calculate the
		 *			normalization factor
		 * @return	the normalization factor */
		float calculateNormalizationFactor(const AABB& aabb) const;

		/** Calculates the contribution of the Aspect Ratio to the cost function
		 *
		 * @param	normalizationFactor the normalization factor
		 * @return	the aspect ratio factor */
		float calculateAspectRatioFactor(float normalizationFactor) const;

		/** Calculates the decimation cost of the edge with the given aspect
		 * ratio and the concavity of the surface of their faces
		 *
		 * @param	concavity the concavity value of the edge
		 * @param	aspectRatio the aspect ratio factor of the edge
		 * @return	the cost value of the edge */
		float calculateDecimationCost(float concavity, float aspectRatio) const;

		/** Adds the vertex 2 and its ancestors to the first one
		 *
		 * @param	vertex1 the first vertex. It's the vertex whose ancestors
		 *			will be updated
		 * @param	vertex2 the second vertex */
		void updateAncestors(
			DualGraphVertex& vertex1, const DualGraphVertex& vertex2
		) const;

		/** Computes the convex surfaces from the partitions of the current
		 * dual graph and the triangulated mesh */
		void computeConvexSurfaces();
	};

}}

#endif		// HACD_H
