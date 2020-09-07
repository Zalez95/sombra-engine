#include <glm/gtx/spline.hpp>
#include "se/animation/CubicSplineAnimations.h"

namespace se::animation {

	AnimationVec3CubicSpline::PrimitiveType AnimationVec3CubicSpline::interpolationFunction(
		const KeyFrame& k1, const KeyFrame& k2, float timePoint
	) const
	{
		float factor = timePoint - k1.timePoint;
		float length = k2.timePoint - k1.timePoint;
		if (length > 0.0f) {
			factor /= length;
		}

		return glm::hermite(k1.point, k1.outTangent, k2.point, k2.inTangent, length);
	}


	AnimationQuatCubicSpline::PrimitiveType AnimationQuatCubicSpline::interpolationFunction(
		const KeyFrame& k1, const KeyFrame& k2, float timePoint
	) const
	{
		float factor = timePoint - k1.timePoint;
		float length = k2.timePoint - k1.timePoint;
		if (length > 0.0f) {
			factor /= length;
		}

		return glm::hermite(k1.point, k1.outTangent, k2.point, k2.inTangent, length);
	}

}
