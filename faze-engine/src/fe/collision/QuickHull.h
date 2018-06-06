#ifndef QUICK_HULL_H
#define QUICK_HULL_H

#include <vector>
#include <glm/glm.hpp>

namespace fe { namespace collision {

	/**
	 * Class QuickHull, TODO:
	 */
	class QuickHull
	{
	private:	// Private
		typedef std::vector<short> Face;

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
		std::vector<glm::vec3> createInitialHull(const std::vector<glm::vec3>& points) const;

		/** TODO: */
		void addFaceToHull(
			const Face& face,
			const std::vector<glm::vec3>& vertices
		);

		/** TODO: */
		void calculateHorizon() {};
	};

}}

#endif		// QUICK_HULL_H
