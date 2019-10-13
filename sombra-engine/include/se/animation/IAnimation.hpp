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
				[](const KeyFrame& k1, const KeyFrame& k2) { return k1.timePoint < k2.timePoint; }
			),
			keyFrame
		);
	}


	template <typename T, typename U>
	T Animation<T, U>::interpolate(float timePoint) const
	{
		auto [k1, k2] = getPreviousAndNextKeyFrames(timePoint);
		return interpolationFunction(k1, k2, timePoint);
	}

// Private functions
	template <typename T, typename U>
	typename Animation<T, U>::KeyFramePair Animation<T, U>::getPreviousAndNextKeyFrames(float timePoint) const
	{
		KeyFramePair ret;

		if (!mKeyFrames.empty()) {
			auto itKeyFrame = std::upper_bound(
				mKeyFrames.begin(), mKeyFrames.end(), timePoint,
				[](float timePoint, const KeyFrame& keyFrame) { return keyFrame.timePoint > timePoint; }
			);
			if (itKeyFrame != mKeyFrames.end()) {
				ret.second = *itKeyFrame;
				if (itKeyFrame != mKeyFrames.begin()) {
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
