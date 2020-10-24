#ifndef CUBIC_SPLINE_ANIMATIONS_H
#define CUBIC_SPLINE_ANIMATIONS_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "IAnimation.h"

namespace se::animation {

	/** A single KeyFrame data */
	struct Vec3CubicSplineKeyFrame
	{
		/** The first tangent used for calculating the spline */
		glm::vec3 inTangent;

		/** The spline vertex used for calculating the spline */
		glm::vec3 point;

		/** The second tangent used for calculating the spline */
		glm::vec3 outTangent;

		/** The time point in seconds since the start of the Animation */
		float timePoint;
	};


	/** A single KeyFrame data */
	struct QuatCubicSplineKeyFrame
	{
		/** The first tangent used for calculating the spline */
		glm::quat inTangent;

		/** The spline vertex used for calculating the spline */
		glm::quat point;

		/** The second tangent used for calculating the spline */
		glm::quat outTangent;

		/** The time point in seconds since the start of the Animation */
		float timePoint;
	};


	/**
	 * Class AnimationQuatCubicSpline, it's an Animation that holds vec3
	 * transformation data. It also implements a cubic spline interpolation
	 * function
	 */
	class AnimationVec3CubicSpline :
		public Animation<glm::vec3, Vec3CubicSplineKeyFrame>
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
		virtual PrimitiveType interpolationFunction(
			const KeyFrame& k1, const KeyFrame& k2, float timePoint
		) const override;

		/** Calculates the time in seconds since the start of the Animation of
		 * the given KeyFrame
		 *
		 * @param	k the KeyFrame to calculate its time point in seconds
		 * @return	the time point in seconds */
		virtual float getTimeInSeconds(const KeyFrame& k) const override
		{ return k.timePoint; };
	};


	/**
	 * Class AnimationQuatCubicSpline, it's an Animation that holds quaternion
	 * transformation data. It also implements a cubic spline interpolation
	 * function
	 */
	class AnimationQuatCubicSpline :
		public Animation<glm::quat, QuatCubicSplineKeyFrame>
	{
	protected:
		/** The cubic spline interpolation function used for calculating the
		 * quat transformation with the Animations KeyFrames and the given
		 * timePoint
		 *
		 * @param	k1 the first KeyFrame data
		 * @param	k2 the second KeyFrame data
		 * @param	timePoint the timePoint used for interpolating the data
		 * @return	the interpolated quat transformation
		 * @note	k2 must be later than k1 */
		virtual PrimitiveType interpolationFunction(
			const KeyFrame& k1, const KeyFrame& k2, float timePoint
		) const override;

		/** Calculates the time in seconds since the start of the Animation of
		 * the given KeyFrame
		 *
		 * @param	k the KeyFrame to calculate its time point in seconds
		 * @return	the time point in seconds */
		virtual float getTimeInSeconds(const KeyFrame& k) const override
		{ return k.timePoint; };
	};

}

#endif		// CUBIC_SPLINE_ANIMATIONS_H
