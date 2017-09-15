#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

#include "ConvexPolyhedron.h"

namespace collision {

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
		 * @param	lengths the lenght in each aixs of the BoundingBox */
		BoundingBox(const glm::vec3& lengths) :
			ConvexPolyhedron(verticesFromLengths(lengths)) {};

		/** Class destructor */
		~BoundingBox() {};
	private:
		/** Calculates the vertices of the BoundingBox from its lenghts in
		 * each axis
		 * 
		 * @param	lengths the lenght in each aixs of the BoundingBox */
		std::vector<glm::vec3> verticesFromLengths(const glm::vec3& lengths)
		{
			std::vector<glm::vec3> ret = {
				glm::vec3(-lengths.x, -lengths.y, -lengths.z) / 2.0f,
				glm::vec3(-lengths.x, -lengths.y,  lengths.z) / 2.0f,
				glm::vec3(-lengths.x,  lengths.y, -lengths.z) / 2.0f,
				glm::vec3(-lengths.x,  lengths.y,  lengths.z) / 2.0f,
				glm::vec3( lengths.x, -lengths.y, -lengths.z) / 2.0f,
				glm::vec3( lengths.x, -lengths.y,  lengths.z) / 2.0f,
				glm::vec3( lengths.x,  lengths.y, -lengths.z) / 2.0f,
				glm::vec3( lengths.x,  lengths.y,  lengths.z) / 2.0f
			};

			return ret;
		};
	};

}

#endif		// BOUNDING_BOX_H
