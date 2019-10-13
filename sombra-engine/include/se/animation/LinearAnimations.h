#ifndef LINEAR_ANIMATIONS_H
#define LINEAR_ANIMATIONS_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "IAnimation.h"

namespace se::animation {

	/**
	 * Class AnimationQuatLinear, it's an Animation that holds vec3
	 * transformation data. It also implements a linear interpolation function
	 */
	class AnimationVec3Linear : public Animation<glm::vec3, float>
	{
	protected:
		/** The linear interpolation function used for calculating the
		 * vec3 transformation with the Animations KeyFrames and the given
		 * timePoint
		 *
		 * @param	k1 the first KeyFrame data
		 * @param	k2 the second KeyFrame data
		 * @param	timePoint the timePoint used for interpolating the data
		 * @return	the interpolated vec3 transformation
		 * @note	k2 must be later than k1 */
		virtual glm::vec3 interpolationFunction(
			const KeyFrame& k1, const KeyFrame& k2, float timePoint
		) const override;
	};


	/**
	 * Class AnimationQuatLinear, it's an Animation that holds quaternion
	 * transformation data. It also implements a linear interpolation function
	 */
	class AnimationQuatLinear : public Animation<glm::quat, float>
	{
	protected:
		/** The linear interpolation function used for calculating the
		 * quat transformation with the Animations KeyFrames and the given
		 * timePoint
		 *
		 * @param	k1 the first KeyFrame data
		 * @param	k2 the second KeyFrame data
		 * @param	timePoint the timePoint used for interpolating the data
		 * @return	the interpolated quat transformation
		 * @note	k2 must be later than k1 */
		virtual glm::quat interpolationFunction(
			const KeyFrame& k1, const KeyFrame& k2, float timePoint
		) const override;
	};

}

#endif		// LINEAR_ANIMATIONS_H
