#ifndef STEP_ANIMATIONS_H
#define STEP_ANIMATIONS_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "IAnimation.h"

namespace se::animation {

	/**
	 * Class AnimationQuatStep, it's an Animation that holds vec3
	 * transformation data. It also implements a step interpolation function
	 */
	class AnimationVec3Step : public Animation<glm::vec3, float>
	{
	protected:
		/** The step interpolation function used for calculating the
		 * vec3 transformation with the Animations KeyFrames and the given
		 * timePoint. The return value of this intepolation function is always
		 * the first of the keyframes.
		 *
		 * @param	k1 the first KeyFrame data
		 * @param	k2 the second KeyFrame data
		 * @param	timePoint the timePoint used for interpolating the data
		 * @return	the interpolated vec3 transformation
		 * @note	k2 must be later than k1 */
		virtual glm::vec3 interpolationFunction(
			const KeyFrame& k1, const KeyFrame& /*k2*/, float /*timePoint*/
		) const override { return k1.transformation; };

		/** Calculates the time in seconds since the start of the Animation of
		 * the given KeyFrame
		 *
		 * @param	k the KeyFrame to calculate its time point in seconds
		 * @return	the time point in seconds */
		virtual float getTimeInSeconds(const KeyFrame& k) const override
		{ return k.timePoint; };
	};


	/**
	 * Class AnimationQuatStep, it's an Animation that holds quaternion
	 * transformation data. It also implements a step interpolation function
	 */
	class AnimationQuatStep : public Animation<glm::quat, float>
	{
	protected:
		/** The step interpolation function used for calculating the
		 * quat transformation with the Animations KeyFrames and the given
		 * timePoint. The return value of this intepolation function is always
		 * the first of the keyframes.
		 *
		 * @param	k1 the first KeyFrame data
		 * @param	k2 the second KeyFrame data
		 * @param	timePoint the timePoint used for interpolating the data
		 * @return	the interpolated quat transformation
		 * @note	k2 must be later than k1 */
		virtual glm::quat interpolationFunction(
			const KeyFrame& k1, const KeyFrame& /*k2*/, float /*timePoint*/
		) const override { return k1.transformation; };

		/** Calculates the time in seconds since the start of the Animation of
		 * the given KeyFrame
		 *
		 * @param	k the KeyFrame to calculate its time point in seconds
		 * @return	the time point in seconds */
		virtual float getTimeInSeconds(const KeyFrame& k) const override
		{ return k.timePoint; };
	};

}

#endif		// STEP_ANIMATIONS_H
