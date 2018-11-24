#ifndef HACD_H
#define HACD_H

#include "AABB.h"
#include "Graph.h"
#include "HalfEdgeMesh.h"

namespace se::collision {

	/**
	 * Class HACD, it's used to calculate the aproximate convex decomposition
	 * of any given HalfEdgeMesh
	 */
	class HACD
	{
	private:	// Nested types
		using DualGraphVertex = GraphVertex< std::vector<int> >;
		using DualGraph = Graph< std::vector<int> >;

		/** Struct QHACDData, it holds the data that the HACD algorithm will
		 * need at each iteration for deciding which Edge of the DualGraph
		 * should be collapsed */
		struct QHACDData
		{
			/** The indices of the Graph Vertices of the Edge */
			int iVertex1, iVertex2;

			/** The cost of merging this Graph Edge */
			float cost;

			/** The concavity of the mesh created from the vertices and their
			 * ancestors */
			float concavity;

			/** Compares the given QHACDData with the current one
			 *
			 * @param	other the other QHACDData to compare
			 * @return	true if the current QHACDData has a higher cost than
			 *			the given one, false otherwise */
			bool operator>(const QHACDData& other) const
			{ return cost > other.cost; };

			/** Compares the given QHACDData with the current one
			 *
			 * @param	other the other QHACDData to compare
			 * @return	true if both QHACDData are the same */
			bool compareVertexIds(const QHACDData& other) const
			{ return iVertex1 == other.iVertex1 || iVertex1 == other.iVertex2
				|| iVertex2 == other.iVertex1 || iVertex2 == other.iVertex2; };
		};

	private:	// Attributes
		/** The maximum concavity of the dual graph edges needed for HACD
		 * algorithm */
		const float mMaximumConcavity;

		/** The epsilon value needed for the comparisons during the HACD
		 * algorithm computation */
		const float mEpsilon;

		/** The precision of the comparison scaled with the size of the
		 * HalfEdgeMesh to calculate */
		float mScaledEpsilon;

		/** The mesh to apply the HACD algorithm. Its faces must be triangles */
		HalfEdgeMesh mMesh;

		/** The normal vectors of the HEFaces of the Mesh to apply the HACD
		 * algorithm */
		NormalMap mFaceNormals;

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

		/** The convex HEMeshes in which the concave mesh has been decomposed */
		std::vector<HalfEdgeMesh> mConvexMeshes;

	public:		// Functions
		/** Creates a new HACD object
		 *
		 * @param	maximumConcavity the maximum concavity needed for HACD
		 *			algorithm
		 * @param	epsilon the epsilon value for the comparisons during
		 *			the HACD algorithm computation */
		HACD(float maximumConcavity, float epsilon) :
			mMaximumConcavity(maximumConcavity), mEpsilon(epsilon) {};

		/** @return	the HalfEdgeMeshes of the HACD */
		const std::vector<HalfEdgeMesh>& getMeshes() const
		{ return mConvexMeshes; };

		/** Decomposes the given mesh into multiple convex ones with the HACD
		 * algorithm
		 *
		 * @param	originalMesh the Half-Edge data structure with the 3D Mesh
		 *			to decompose with the HACD algorithm */
		void calculate(const HalfEdgeMesh& originalMesh);

		/** Resets the HACD data for the next calculations */
		void resetData();
	private:
		/** Initializes all the data needed for the HACD algorithm computation
		 *
		 * @param	originalMesh the Half-Edge data structure with the 3D Mesh
		 *			to decompose with the HACD algorithm */
		void initData(const HalfEdgeMesh& originalMesh);

		/** Creates and return a new QHACDData created from the given vertices
		 * and the HACD data
		 *
		 * @param	vertex1 the first of the Graph vertices
		 * @param	vertex2 the second of the Graph vertices
		 * @return	the new QHACDData */
		QHACDData createQHACDData(
			const DualGraphVertex& vertex1, const DualGraphVertex& vertex2
		) const;

		/** Adds the vertex 2 and its ancestors to the first one
		 *
		 * @param	vertex1 the first vertex. It's the vertex whose ancestors
		 *			will be updated
		 * @param	vertex2 the second vertex */
		static void updateAncestors(
			DualGraphVertex& vertex1, const DualGraphVertex& vertex2
		);

		/** Computes the convex surfaces from the partitions of the current
		 * dual graph and the triangulated mesh */
		void computeConvexSurfaces();

		/** Creates the initial Dual Graph with the triangles of the Mesh.
 		 *
		 * @param	meshData the Half-Edge Mesh from which we want to create
		 *			the dual graph
		 * @return	the dual graph of the mesh */
		DualGraph createDualGraph(const HalfEdgeMesh& meshData) const;

		/** Calculates the normalization factor as the length of the diagonal
		 * of the MeshCollider's AABB
		 *
		 * @param	aabb the AABB with which we want to calculate the
		 *			normalization factor
		 * @return	the normalization factor */
		static float calculateNormalizationFactor(const AABB& aabb);

		/** Calculates the contribution of the Aspect Ratio to the cost function
		 *
		 * @param	normalizationFactor the normalization factor
		 * @return	the aspect ratio factor */
		float calculateAspectRatioFactor(float normalizationFactor) const;

		/** Calculates the HEFace indices of the surface created from the given
		 * Graph vertices and their ancestors
		 *
		 * @param	iVertex1 the first Graph vertex
		 * @param	iVertex2 the second Graph vertex
		 * @return	the HEFace indices of the surface */
		static std::vector<int> calculateSurfaceFaceIndices(
			const DualGraphVertex& vertex1, const DualGraphVertex& vertex2
		);

		/** Creates a new HEMesh from the provided HEFaces of the given HEMesh
		 *
		 * @param	iFaces the indices of the HEFaces of the meshData
		 *			we want to create the new HEMesh
		 * @param	meshData the HalfEdgeMesh that holds the HEFaces
		 * @param	faceNormals a map with the normal vectors of the meshData
		 *			HEFaces
		 * @return	a pair with new HEMesh and its face normals */
		static std::pair<HalfEdgeMesh, NormalMap> getMeshFromIndices(
			const std::vector<int>& iFaces,
			const HalfEdgeMesh& meshData, const NormalMap& faceNormals
		);

		/** Calculates the concavity of the given HalfEdgeMesh
		 *
		 * @param	 originalMesh the HalfEdgeMesh to calculate its concavity
		 * @param	faceNormals a map with the normal vectors of the
		 *			originalMesh HEFaces
		 * @param	convexHullMesh the convex hull HalfEdgeMesh of the
		 *			originalMesh
		 * @param	convexHullNormals a map with the normal vector of the
		 *			convexHullMesh HEFaces
		 * @return	the concavity of the surface */
		float calculateConcavity(
			const HalfEdgeMesh& originalMesh,
			const NormalMap& originalMeshNormals,
			const HalfEdgeMesh& convexHullMesh,
			const NormalMap& convexHullNormals
		) const;

		/** Calculates the maximum concavity of the given 2D HalfEdgeMesh as the
		 * square root of the difference between the areas of the convex hull
		 * and the original HalfEdgeMesh
		 *
		 * @param	originalMesh the HalfEdgeMesh to calculate its concavity
		 * @param	convexHullMesh the convex hull HalfEdgeMesh of the
		 *			originalMesh
		 * @return	the concavity of the surface
		 * @note	both HalfEdgeMeshes must have triangular HEFaces */
		float calculateConcavity2D(
			const HalfEdgeMesh& originalMesh, const HalfEdgeMesh& convexHullMesh
		) const;

		/** Calculates the maximum concavity of the given 3D HalfEdgeMesh as the
		 * maximum distance from a point on its surface to its convex hull.
		 *
		 * @param	originalMesh the HalfEdgeMesh to calculate its concavity
		 * @param	originalMeshNormals a map with the normal vectors of the
		 *			originalMesh HEFaces
		 * @param	convexHullMesh the convex hull HalfEdgeMesh of the
		 *			originalMesh
		 * @param	convexHullNormals a map with the normal vector of the
		 *			convexHullMesh HEFaces
		 * @return	the concavity of the surface */
		float calculateConcavity3D(
			const HalfEdgeMesh& originalMesh,
			const NormalMap& originalMeshNormals,
			const HalfEdgeMesh& convexHullMesh,
			const NormalMap& convexHullNormals
		) const;

		/** Calculate the aspect ratio of the given HEMesh
		 *
		 * @param	meshData the HalfEdgeMesh to calculate its aspect ratio
		 * @return	the aspect ratio of the HEMesh */
		static float calculateAspectRatio(const HalfEdgeMesh& meshData);

		/** Calculates the decimation cost of the edge with the given aspect
		 * ratio and the concavity of the surface of their faces
		 *
		 * @param	concavity the concavity value of the edge
		 * @param	aspectRatio the aspect ratio factor of the edge
		 * @return	the cost value of the edge */
		float calculateDecimationCost(float concavity, float aspectRatio) const;

		/** Calculates the intersection of the given internal point in the given
		 * direction with the convex HalfEdgeMesh with the furthest distance
		 * from it
		 *
		 * @param	meshData the convex HalfEdgeMesh
		 * @param	faceNormals the normal vector of the mesh HEFaces
		 * @param	origin the coordinates of the internal point
		 * @param	direction the direction towards we want to calculate the
		 *			intersection
		 * @return	a pair with a bool that indicates if the intersection could
		 *			be calculated and the coordinates of the intersection */
		std::pair<bool, glm::vec3> getInternalIntersection(
			const HalfEdgeMesh& meshData, const NormalMap& faceNormals,
			const glm::vec3& origin, const glm::vec3& direction
		) const;

		/** Checks if the given point is located between the given loop edges
		 *
		 * @param	meshData the HalfEdgeMesh where the HEEdges are located
		 * @param	iInitialEdge the index of the starting HEEdge of the loop
		 * @param	loopNormal the normal vector of the HEEdge loop
		 * @param	point the coordinates of the point to check
		 * @return	true if the point is between the HEEdges, false otherwise */
		bool isPointBetweenHEEdges(
			const HalfEdgeMesh& meshData, int iInitialEdge,
			const glm::vec3& loopNormal, const glm::vec3& point
		) const;
	};

}

#endif		// HACD_H