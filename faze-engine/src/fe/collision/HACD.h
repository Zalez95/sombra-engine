#ifndef HACD_H
#define HACD_H

#include <vector>
#include <glm/glm.hpp>

namespace fe { namespace collision {

	/**
	 * Class HACD, TODO:
	 */
	class HACD
	{
	private:	// Nested types
		typedef std::vector<std::vector<bool>> EdgeMatrix;

	private:	// Attributes
		/** The minimum concavity needed for HACD algorithm */
		static constexpr float sMinimumConcavity = 0.5f;

	public:		// Functions
		/** Creates a new MeshCollider located at the origin of
		 * coordinates
		 *
		 * @param	vertices the vertices of the MeshCollider in local
		 *			space
		 * @param	indices the indices of the triangle mesh of the
		 *			MeshCollider */
		HACD(
			const std::vector<glm::vec3>& vertices,
			const std::vector<unsigned short>& indices
		);

		/** Class destructor */
		virtual ~HACD() {};
	private:
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

#endif		// HACD_H
