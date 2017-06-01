#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

#include <vector>
#include "Collider.h"

namespace physics {

	/**
	 * Class BoundingBox, an Bounding Box is a bounding volume with the shape
	 * of a box
	 */
	class BoundingBox : public Collider
	{
	private:	// Attributes
		/** The lenght in each aixs of the BoundingBox */
		glm::vec3 mLengths;

		/** The coordinates in world space of the BoudingBox's vertices */
		std::vector<glm::vec3> mVertices;

	public:		// Functions
		/** Creates a new BoundingBox located at the origin of
		 * coordinates
		 * 
		 * @param	lengths the lenght in each aixs of the BoundingBox */
		BoundingBox(const glm::vec3& lengths) :
			mLengths(lengths), mVertices(8) {};

		/** Class destructor */
		~BoundingBox() {};

		/** @return the coordinates of the BoundingBox's vertices in world
		 *			space */
		inline std::vector<glm::vec3> getVertices() const
		{ return mVertices; };

		/** Updates the position of the BoundingBox with the data of the
		 * given transformation matrix
		 * 
		 * @param	transforms the transformations matrix used to set the
		 *			position and rotation of the BoundingBox */
		virtual void setTransforms(const glm::mat4& transforms);
	};

}

#endif		// BOUNDING_BOX_H
