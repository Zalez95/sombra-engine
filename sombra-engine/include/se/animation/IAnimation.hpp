#ifndef I_ANIMATION_HPP
#define I_ANIMATION_HPP

#include <algorithm>

namespace se::animation {

	template <typename T, typename U>
	void Animation<T, U>::addKeyFrame(const KeyFrame& keyFrame)
	{
		mKeyFrames.insert(
			std::lower_bound(
				mKeyFrames.begin(), mKeyFrames.end(), keyFrame,
				[this](const KeyFrame& k1, const KeyFrame& k2) { return getTimeInSeconds(k1) < getTimeInSeconds(k2); }
			),
			keyFrame
		);
	}


	template <typename T, typename U>
	typename IAnimation<T>::PrimitiveType Animation<T, U>::interpolate(float timePoint) const
	{
		auto [k1, k2] = getPreviousAndNextKeyFrames(timePoint);
		return interpolationFunction(k1, k2, timePoint);
	}


	template <typename T, typename U>
	float Animation<T, U>::getLength() const
	{
		if (mKeyFrames.empty()) {
			return 0.0f;
		}
		else {
			return getTimeInSeconds( mKeyFrames.back() );
		}
	}

// Private functions
	template <typename T, typename U>
	typename Animation<T, U>::KeyFramePair Animation<T, U>::getPreviousAndNextKeyFrames(float timePoint) const
	{
		KeyFramePair ret;

		if (!mKeyFrames.empty()) {
			auto itKeyFrame = std::upper_bound(
				mKeyFrames.begin(), mKeyFrames.end(), timePoint,
				[this](float timePoint, const KeyFrame& keyFrame) { return timePoint < getTimeInSeconds(keyFrame); }
			);
			if (itKeyFrame != mKeyFrames.end()) {
				if (itKeyFrame == mKeyFrames.begin()) {
					ret.first = ret.second = mKeyFrames.front();
				}
				else {
					ret.second = *itKeyFrame;
					--itKeyFrame;
					ret.first = *itKeyFrame;
				}
			}
			else {
				ret.first = ret.second = mKeyFrames.back();
			}
		}

		return ret;
	}

}

#endif		// I_ANIMATION_HPP
