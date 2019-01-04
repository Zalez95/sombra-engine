#include <cassert>
#include <algorithm>
#include "se/animation/Animation.h"

namespace se::animation {

	KeyFrame::KeyFrame() :
		scale(1.0f), rotation(1.0f, glm::vec3(0.0f)), translation(0.0f), timePoint(0.0f) {}


	KeyFrame::KeyFrame(const glm::vec3& scale, const glm::quat& rotation, const glm::vec3& translation, float timePoint) :
		scale(scale), rotation(rotation), translation(translation), timePoint(timePoint) {}


	Animation::Animation(const Bone& bone, float length, bool loop) :
		mInitialBone(bone), mLength(length), mLoopAnimation(loop),
		mAccumulatedTime(0.0f), mCurrentBone(mInitialBone) {}


	void Animation::addKeyFrame(const KeyFrame& keyFrame)
	{
		mKeyFrames.insert(
			std::lower_bound(
				mKeyFrames.begin(), mKeyFrames.end(), keyFrame,
				[](const KeyFrame& k1, const KeyFrame& k2) { return k1.timePoint < k2.timePoint; }
			),
			keyFrame
		);
	}


	KeyFrame keyFrameLinearInterpolation(const KeyFrame& k1, const KeyFrame& k2, float factor)
	{
		assert(factor >= 0.0f && factor <= 1.0f && "The factor must be in the range [0, 1]");

		return KeyFrame {
			glm::mix(k1.scale, k2.scale, factor),
			glm::normalize(glm::slerp(k1.rotation, k2.rotation, factor)),
			glm::mix(k1.translation, k2.translation, factor),
			(1.0f - factor) * k1.timePoint + factor * k2.timePoint
		};
	}

}
