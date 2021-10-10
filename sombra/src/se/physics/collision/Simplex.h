#ifndef SIMPLEX_H
#define SIMPLEX_H

#include "se/utils/FixedVector.h"
#include "SupportPoint.h"

namespace se::physics {

	using Simplex = utils::FixedVector<SupportPoint, 4>;


	/** Checks if the origin of coordinates in CSO space is inside the
	 * given simplex
	 *
	 * @param	simplex the simplex to test
	 * @param	epsilon the epsilon value used for comparisons
	 * @return	true if it's inside, false otherwise */
	bool isOriginInside(const Simplex& simplex, float epsilon);


	/** Checks if there is a point in the given simplex with the same
	 * coordinates in CSO space
	 *
	 * @param	simplex the simplex to test
	 * @param	point the point to test in CSO space
	 * @param	epsilon the epsilon value used for the comparisons
	 * @return	true if there is a point already inside the simplex with the
	 *			given coordinates in CSO space, false otherwise */
	bool isClose(const Simplex& simplex, const glm::vec3& point, float epsilon);

}

#endif		// SIMPLEX_H
