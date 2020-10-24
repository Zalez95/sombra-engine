#ifndef I_ANIMATION_H
#define I_ANIMATION_H

#include <vector>

namespace se::animation {

	/**
	 * Class IAnimation, it holds the set of KeyFrames of an Animation
	 */
	template <typename T>
	class IAnimation
	{
	public:		// Nested types
		using PrimitiveType = T;

	public:		// Functions
		/** Class destructor */
		virtual ~IAnimation() = default;

		/** Calculates an interpolated transformation by applying an
		 * interpolation function to the data of the KeyFrames
		 *
		 * @param	timePoint the time point used for interpolating the data
		 * @return	the interpolated transformation */
		virtual PrimitiveType interpolate(float timePoint) const = 0;

		/** @return	the length in seconds of the animation */
		virtual float getLength() const = 0;
	};


	/**
	 * Class Animation, it holds the set of KeyFrames of an Animation
	 * @tparam	T is the primitive type that is going to be interpolated
	 * @tparam	U holds the data of each KeyFrame needed for calculating the
	 *			interpolation
	 */
	template <typename T, typename U>
	class Animation : public IAnimation<T>
	{
	public:		// Nested types
		using KeyFrame = U;
		using KeyFramePair = std::pair<KeyFrame, KeyFrame>;

	protected:	// Attributes
		/** The KeyFrames raw data sorted by their timePoint */
		std::vector<KeyFrame> mKeyFrames;

	public:		// Functions
		/** Class destructor */
		virtual ~Animation() = default;

		/** Adds a new KeyFrame to the Animation
		 *
		 * @param	keyFrame the KeyFrame to add to the Animation */
		void addKeyFrame(const KeyFrame& keyFrame);

		/** Calculates an interpolated transformation by applying an
		 * interpolation function to the data of the KeyFrames
		 *
		 * @param	timePoint the timePoint used for interpolating the data
		 * @return	the interpolated transformation */
		virtual typename IAnimation<T>::PrimitiveType interpolate(
			float timePoint
		) const override;

		/** @return	the length in seconds of the animation */
		virtual float getLength() const override;
	protected:
		/** The interpolation function used for calculating the transformation
		 * with the Animations KeyFrames and the given timePoint
		 *
		 * @param	k1 the first KeyFrame data
		 * @param	k2 the second KeyFrame data
		 * @param	timePoint the timePoint used for interpolating the data
		 * @return	the interpolated transformation
		 * @note	k2 must be later than k1 */
		virtual typename IAnimation<T>::PrimitiveType interpolationFunction(
			const KeyFrame& k1, const KeyFrame& k2, float timePoint
		) const = 0;

		/** Calculates the time in seconds since the start of the Animation of
		 * the given KeyFrame
		 *
		 * @param	k the KeyFrame to calculate its time point in seconds
		 * @return	the time point in seconds */
		virtual float getTimeInSeconds(const KeyFrame& k) const = 0;

		/** Returns the previous and next KeyFrames to the given time point
		 *
		 * @param	timePoint the time in seconds with which we want to search
		 *			the KeyFrames
		 * @return	a pair with the previous and next KeyFrames */
		KeyFramePair getPreviousAndNextKeyFrames(float timePoint) const;
	};

}

#include "IAnimation.hpp"

#endif		// I_ANIMATION_H
