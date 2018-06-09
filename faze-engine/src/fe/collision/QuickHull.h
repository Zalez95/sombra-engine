#ifndef QUICK_HULL_H
#define QUICK_HULL_H

#include <vector>
#include <glm/glm.hpp>
#include "MeshAdjacencyData.h"

namespace fe { namespace collision {

	/**
	 * Class QuickHull, TODO:
	 */
	class QuickHull
	{
	private:	// Attributes
		MeshAdjacencyData mMesh;

	public:		// Functions
		/** Creates a new QuickHull
		 *
		 * @param	vertices the vertices of the Mesh to calculate the
		 *			convex hull with
		 * @param	indices the indices of the faces of the triangle Mesh to
		 *			calculate the convex hull with */
		QuickHull(
			const std::vector<glm::vec3>& vertices,
			const std::vector<unsigned short>& indices
		);

		/** Class destructor */
		~QuickHull() {};
	private:
		/** Calculates the initial simplex needed for calculating the QuickHull
		 * algorithm from the given points
		 *
		 * @param	points the points with which we will calculate the initial
		 *			simplex
		 * @note	the number of points must be bigger than 3
		 * @return	the initial simplex of the QuickHull algorithm */
		std::vector<glm::vec3> createInitialHull(
			const std::vector<glm::vec3>& points
		) const;

		/** Calculates the vertices that are outside Hull by the given face
		 *
		 * @param	face the triangular face used to determine which vertex is
		 *			outside or inside the convex hull
		 * @param	vertices the vertices to add
		 * @return	the vertices outside the convex hull */
		std::vector<glm::vec3> getVerticesOutside(
			const Face& face,
			const std::vector<glm::vec3>& vertices
		) const;

		/** TODO:
		 *
		 * @param	face
		 * @param	vertices
		 * @return	the furthest point */
		glm::vec3 getFurthestPoint(
			const Face& face,
			const std::vector<glm::vec3>& vertices
		) const;

		/** Calculates the boundary of the convex hull as seen from the given
		 * eyePoint
		 * 
		 * @param	eyePoint the 3D coordinates of the point
		 * @param	face the current face
		 * @param	horizon the list of edges where we are going to store the
		 *			boundary of the convex hull */
		void calculateHorizon(
			const glm::vec3& eyePoint, const Face& face,
			const std::vector<Edge>& horizon
		) const;
	};

}}

#endif		// QUICK_HULL_H
