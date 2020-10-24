#include "se/animation/LinearAnimations.h"

namespace se::animation {

	AnimationVec3Linear::PrimitiveType AnimationVec3Linear::interpolationFunction(
		const KeyFrame& k1, const KeyFrame& k2, float timePoint
	) const
	{
		float factor = timePoint - k1.timePoint;
		float length = k2.timePoint - k1.timePoint;
		if (length > 0.0f) {
			factor /= length;
		}

		return glm::mix(k1.transformation, k2.transformation, factor);
	}


	AnimationQuatLinear::PrimitiveType AnimationQuatLinear::interpolationFunction(
		const KeyFrame& k1, const KeyFrame& k2, float timePoint
	) const
	{
		float factor = timePoint - k1.timePoint;
		float length = k2.timePoint - k1.timePoint;
		if (length > 0.0f) {
			factor /= length;
		}

		return glm::normalize(glm::slerp(k1.transformation, k2.transformation, factor));
	}

}
