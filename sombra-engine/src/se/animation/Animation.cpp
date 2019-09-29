#include <cassert>
#include <algorithm>
#include "se/animation/Animation.h"

namespace se::animation {

	KeyFrame::KeyFrame() :
		translation(0.0f), rotation(1.0f, glm::vec3(0.0f)), scale(1.0f), timePoint(0.0f) {}


	KeyFrame::KeyFrame(const glm::vec3& translation, const glm::quat& rotation, const glm::vec3& scale, float timePoint) :
		translation(translation), rotation(rotation), scale(scale), timePoint(timePoint) {}


	Animation::Animation(float length, bool loop) :
		mLength(length), mLoopAnimation(loop), mAccumulatedTime(0.0f) {}


	void Animation::addKeyFrame(SceneNode* node, const KeyFrame& keyFrame)
	{
		KeyFrames& keyFrames = mNodeKeyFrames[node];
		keyFrames.insert(
			std::lower_bound(
				keyFrames.begin(), keyFrames.end(), keyFrame,
				[](const KeyFrame& k1, const KeyFrame& k2) { return k1.timePoint < k2.timePoint; }
			),
			keyFrame
		);
	}


	KeyFrame keyFrameLinearInterpolation(const KeyFrame& k1, const KeyFrame& k2, float factor)
	{
		assert(factor >= 0.0f && factor <= 1.0f && "The factor must be in the range [0, 1]");

		return KeyFrame {
			glm::mix(k1.translation, k2.translation, factor),
			glm::normalize(glm::slerp(k1.rotation, k2.rotation, factor)),
			glm::mix(k1.scale, k2.scale, factor),
			(1.0f - factor) * k1.timePoint + factor * k2.timePoint
		};
	}

}
