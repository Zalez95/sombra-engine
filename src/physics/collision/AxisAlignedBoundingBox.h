#ifndef AXIS_ALIGNED_BOUNDING_BOX_H
#define AXIS_ALIGNED_BOUNDING_BOX_H

#include "Collider.h"
#include <vector>

namespace physics {

	/**
	 * Class AxisAlignedBoundingBox, an Axis-Aligned Bounding Box (AABB) is
	 * a bounding volume with the shape of a box
	 */
	class AxisAlignedBoundingBox : public Collider
	{
	private:	// Attributes
		/** The lenght in each aixs of the BoundingBox */
		glm::vec3 mLengths;

		/** The coordinates in world space of the BoudingBox's vertices */
		std::vector<glm::vec3> mVertices;

	public:		// Functions
		/** Creates a new AxisAlignedBoundingBox located at the origin of
		 * coordinates
		 * 
		 * @param	lengths the lenght in each aixs of the BoundingBox */
		AxisAlignedBoundingBox(const glm::vec3& lengths) :
			mLengths(lengths), mVertices(8) {};

		/** Class destructor */
		~AxisAlignedBoundingBox() {};

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

#endif		// AXIS_ALIGNED_BOUNDING_BOX_H
