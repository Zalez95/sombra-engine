#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

#include "ConvexPolyhedron.h"

namespace se::collision {

	/**
	 * Class BoundingBox, a Bounding Box is a convex bounding volume with the
	 * shape of a box
	 */
	class BoundingBox : public ConvexPolyhedron
	{
	public:		// Functions
		/** Creates a new BoundingBox located at the origin of
		 * coordinates
		 *
		 * @param	lengths the lenght in each axis of the BoundingBox */
		BoundingBox(const glm::vec3& lengths) :
			ConvexPolyhedron(meshFromLengths(lengths)) {};
	private:
		/** Calculates the HalfEdgeMesh of the BoundingBox from its lenghts in
		 * each axis
		 *
		 * @param	lengths the lenght in each aixs of the BoundingBox
		 * @return	the HalfEdgeMesh structure with the Mesh of the
		 *			BoundingBox */
		HalfEdgeMesh meshFromLengths(const glm::vec3& lengths) const;
	};

}

#endif		// BOUNDING_BOX_H
