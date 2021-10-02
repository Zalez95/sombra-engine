#ifndef SUPPORT_POINT_H
#define SUPPORT_POINT_H

#include <glm/glm.hpp>

namespace se::physics {

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
		/** Creates a new SupportPoint */
		SupportPoint();

		/** Creates a new SupportPoint
		 *
		 * @param	worldPosition1 the world position of the first point
		 * @param	localPosition1 the local position of the first point
		 * @param	worldPosition2 the world position of the second point
		 * @param	localPosition2 the local position of the second point */
		SupportPoint(
			const glm::vec3& worldPosition1, const glm::vec3& localPosition1,
			const glm::vec3& worldPosition2, const glm::vec3& localPosition2
		);

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

		/** Compares the given SupportPoints
		 *
		 * @param	sp1 the first SupportPoint to compare
		 * @param	sp2 the second SupportPoint to compare
		 * @return	true if the both SupportPoints are the same, false
		 *			otherwise */
		friend bool operator==(
			const SupportPoint& sp1, const SupportPoint& sp2
		);

		/** @return	the SupportPoint's coordinates in the Configuration Space
		 *			Object */
		glm::vec3 getCSOPosition() const { return mCSOPosition; };

		/** Returns the coordinates in local space of the SupportPoint relative
		 * to the Collider located at the given index
		 *
		 * @param	second the flag used to select the local position to return
		 * @return	the position in local space of the SupportPoint relative to
		 *			the second Collider if the flag is true, or relative to the
		 *			first one of the flag is false */
		glm::vec3 getLocalPosition(bool second) const
		{ return mLocalPosition[second]; };

		/** Returns the coordinates in world space of the SupportPoint relative
		 * to the Collider located at the given index
		 *
		 * @param	second the flag used to select the world position to return

		 * @return	the position in world space of the SupportPoint relative to
		 *			the second Collider if the flag is true, or relative to the
		 *			first one of the flag is false */
		glm::vec3 getWorldPosition(bool second) const
		{ return mWorldPosition[second]; };
	};

}

#endif		// SUPPORT_POINT_H
