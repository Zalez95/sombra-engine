#ifndef MESH_COLLIDER_H
#define MESH_COLLIDER_H

#include "ConcaveCollider.h"
#include "ConvexPolyhedron.h"

namespace fe { namespace collision {

	/**
	 * The algorithm used to split the ConcaveCollider in multiple
	 * ConvexColliders
	 */
	enum class ConvexStrategy
	{
		/** QuickHull algorithm used to calculate the Convex Hull */
		QuickHull,
		/** HACD algorithm used to split a ConcaveCollider in multiple
		 * ConvexColliders */
		HACD
	};


	/**
	 * Class MeshCollider, it's a Collider with a concave shape which will be
	 * stored as a triangle Mesh.
	 */
	class MeshCollider : public ConcaveCollider
	{
	private:	// Nested types
		typedef std::vector<std::vector<bool>> EdgeMatrix;

	private:	// Attributes
		/** The minimum concavity needed for HACD algorithm */
		static constexpr float sMinimumConcavity = 0.5f;

		/** The multiple pieces in which the ConcaveCollider is splited */
		std::vector<ConvexPolyhedron> mConvexParts;

		/** The transformation matrix of the MeshCollider */
		glm::mat4 mTransformsMatrix;

		/** The AABB of the MeshCollider */
		AABB mAABB;

	public:		// Functions
		/** Creates a new MeshCollider located at the origin of
		 * coordinates
		 *
		 * @param	vertices the vertices of the MeshCollider in local
		 *			space
		 * @param	indices the indices of the triangle mesh of the
		 *			MeshCollider */
		MeshCollider(
			const std::vector<glm::vec3>& vertices,
			const std::vector<unsigned short>& indices,
			ConvexStrategy strategy
		);

		/** Class destructor */
		virtual ~MeshCollider() {};

		/** Updates the scale, translation and orientation of the MeshCollider
		 * with the given transformations matrix
		 *
		 * @param	transforms the transformations matrix used to update the
		 *			scale, translation and orientation of the MeshCollider */
		virtual void setTransforms(const glm::mat4& transforms);

		/** @return	the transformations matrix currently applied to the
		 *			MeshCollider */
		inline virtual glm::mat4 getTransforms() const
		{ return mTransformsMatrix; };

		/** @return the Axis Aligned Bounding Box that contains the
		 *			MeshCollider */
		inline virtual AABB getAABB() const { return mAABB; };

		/** @return a set with the posible overlaping parts of the Collider
		 * with the given AABB
		 *
		 * @param	aabb the AABB to compare
		 * @return	a set with the pointers to the Convex parts of the collider
		 *			that could be overlaping with the given AABB */
		virtual std::vector<const ConvexCollider*> getOverlapingParts(
			const AABB& aabb
		) const;
	private:
		/** Calculates the AABB of the MeshCollider */
		void calculateAABB();

		/** TODO: */
		std::vector<glm::vec3> doQuickHull(
			const std::vector<glm::vec3>& points
		) const;

		/** Calculates the initial simplex needed for calculating the QuickHull
		 * algorithm from the given points
		 *
		 * @param	points the points with which we will calculate the initial simplex
		 * @note	the number of points must be bigger than 3
		 * @return	the initial simplex of the QuickHull algorithm */
		std::vector<glm::vec3> createInitialHull(
			const std::vector<glm::vec3>& points
		) const;

		/** Creates the initial Dual Graph with the triangles of the Mesh.
		 * Each vertex in this graph is a triangle in our mesh, and each vertex
		 * in the graph is connected to another one if the triangle
		 * corresponging to that vertex shares an edge with another triangle.
 		 *
		 * @return	a square matrix with the initial connections (Edges) between
		 *			the graph vertices */
		EdgeMatrix createDualGraph(const std::vector<int>& indices) const;

		/** Collapses the given edges into one by removing the edges of the
		 * second point and adding them to the first one
		 *
		 * @param	v1 the index of the graph vertex where v2 is going to be
		 *			collapsed
		 * @param	v2 the index of the graph vertex to collapse
		 * @param	graphEdges the square matrix that holds the edges of the
		 *			graph */
		void halfEdgeCollapse(int v1, int v2, EdgeMatrix& graphEdges) const;

		/** Calculates the cost function of the given edge with the aspect ratio
		 * and the concavity of the surface resulting from the unification of
		 * the given graph vertices and its ancestors
		 *
		 * @param	v1 the first graph vertex
		 * @param	v2 the second graph vertex
		 * @param	vertices the 3D coordinates of the vertices of the mesh
		 * @param	indices the indices of the triangle faces of the mesh
		 * @return	the cost value of the edge between the given vertices */
		float calculateCost(
			int v1, int v2,
			const std::vector<glm::vec3>& vertices,
			const std::vector<unsigned short>& indices
		) const;

		/** Calculates the concavity of the given surface
		 *
		 * @param	v1 the first graph vertex
		 * @param	v2 the second graph vertex
		 * @return	the concavity value of the given surface */
		float calculateConcavity(int v1, int v2) const;

		/** Calculate the aspect ratio of the surface resulting of the union of
		 * the triangles t1 and t2
		 *
		 * @param	t1 the index of the first triangle
		 * @param	t2 the index of the second triangle
		 * @param	vertices the 3D coordinates of the vertices of the mesh
		 * @param	indices the indices of the triangle faces of the mesh
		 * @return	the aspect ratio of the surface */
		float calculateAspectRatio(
			int t1, int t2,
			const std::vector<glm::vec3>& vertices,
			const std::vector<unsigned short>& indices
		) const;

		/** Calculates the normalization factor as the length of the diagonal
		 * of the MeshCollider's AABB */
		float calculateNormalizationFactor() const;

		/** Calculates the contribution of the Aspect Ratio to the cost function
		 *
		 * @param	normalizationFactor the normalization factor
		 * @return	the aspect ratio factor */
		float calculateAspectRatioFactor(float normalizationFactor) const;
	};

}}

#endif		// MESH_COLLIDER_H
