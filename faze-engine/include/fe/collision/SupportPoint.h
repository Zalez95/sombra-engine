#ifndef SUPPORT_POINT_H
#define SUPPORT_POINT_H

#include <glm/glm.hpp>

namespace fe { namespace collision {

	class ConvexCollider;


	/**
	 * Class SupportPoint, it's a class used to map local, world and CSO
	 * coordinates for the GJK and EPA algorithms
	 */
	class SupportPoint
	{
	private:	// Attributes
		/** The SupportPoint's coordinates in the Configuration Space Object */
		glm::vec3 mCSOPosition;

		/** The coordinates of the SupportPoint relative to each of the
		 * ConvexColliders in world space */
		glm::vec3 mWorldPosition[2];

		/** The coordinates of the SupportPoint relative to each of the
		 * ConvexColliders in local space */
		glm::vec3 mLocalPosition[2];

	public:		// Functions
		/** Creates a new SupportPoint as the furthest point in the given
		 * direction inside the Minkowski Difference (or the Configuration
		 * Space Object) of the given colliders
		 *
		 * @param	collider1 the first ConvexCollider with which we want to
		 *			calculate the SupportPoint
		 * @param	collider2 the second ConvexCollider with which we want to
		 *			calculate the SupportPoint
		 * @param	searchDir the direction to search the SupportPoint */
		SupportPoint(
			const ConvexCollider& collider1, const ConvexCollider& collider2,
			const glm::vec3& searchDir
		);

		/** Class destructor */
		~SupportPoint() {};

		/** @return	true if the given SupportPoint is the same than the given
		 *			one, false otherwise */
		bool operator==(const SupportPoint& other) const;

		/** @return	the SupportPoint's coordinates in the Configuration Space
		 *			Object */
		inline glm::vec3 getCSOPosition() const { return mCSOPosition; };

		/** Returns the coordinates in local space of the SupportPoint relative
		 * to the Collider located at the given index
		 *
		 * @param	i the first or the second collider given in the constructor
		 * @return	the coordinates of the SupportPoint in local space */
		inline glm::vec3 getLocalPosition(size_t i) const
		{ return mLocalPosition[i]; };

		/** Returns the coordinates in world space of the SupportPoint relative
		 * to the Collider located at the given index
		 *
		 * @param	i the first or the second collider given in the constructor
		 * @return	the coordinates of the SupportPoint in world space */
		inline glm::vec3 getWorldPosition(size_t i) const
		{ return mWorldPosition[i]; };
	};

}}

#endif		// SUPPORT_POINT_H
