#ifndef CONVEX_POLYHEDRON_H
#define CONVEX_POLYHEDRON_H

#include <vector>
#include "Collider.h"

namespace physics {

	/**
	 * Class ConvexPolyhedron, it's a Collider whose vertices form a convex
	 * shape
	 */
	class ConvexPolyhedron : public Collider
	{
	protected:	// Attributes
		/** The coordinates in local space of the ConvexPolyhedron's
		 * vertices */
		std::vector<glm::vec3> mVertices;

		/** The coordinates in world space of the ConvexPolyhedron's
		 * vertices */
		std::vector<glm::vec3> mVerticesWorld;

	public:		// Functions
		/** Creates a new ConvexPolyhedron located at the origin of
		 * coordinates
		 * 
		 * @param	vertices the vertices of the ConvexPolyhedron in local
		 *			space */
		ConvexPolyhedron(const std::vector<glm::vec3>& vertices);

		/** Class destructor */
		~ConvexPolyhedron() {};

		/** @return the coordinates of the ConvexPolyhedron's vertices in
		 *			world space */
		inline std::vector<glm::vec3> getVertices() const
		{ return mVerticesWorld; };

		/** Updates the position of the ConvexPolyhedron with the data of the
		 * given transformation matrix
		 * 
		 * @param	transforms the transformations matrix used to set the
		 *			position and rotation of the ConvexPolyhedron */
		virtual void setTransforms(const glm::mat4& transforms);

		/** @return the Axis Aligned Bounding Box that contains the
		 *			ConvexPolyhedron */
		virtual AABB getAABB() const;
	};

}

#endif		// CONVEX_POLYHEDRON_H
