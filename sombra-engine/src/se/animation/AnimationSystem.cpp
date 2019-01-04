#include <algorithm>
#include "se/animation/AnimationSystem.h"

namespace se::animation {

	void AnimationSystem::addAnimation(Animation* animation)
	{
		if (animation) {
			mAnimations.push_back(animation);
		}
	}


	void AnimationSystem::removeAnimation(Animation* animation)
	{
		mAnimations.erase(
			std::remove(mAnimations.begin(), mAnimations.end(), animation),
			mAnimations.end()
		);
	}


	void AnimationSystem::update(float delta)
	{
		for (Animation* animation : mAnimations) {
			updateAnimation(*animation, delta);
		}
	}

// Private functions
	void AnimationSystem::updateAnimation(Animation& animation, float delta)
	{
		// Calculate the time since the start of the animation cycle
		float timeSinceStart = (animation.mLength <= 0.0f)? 0.0f :
			(animation.mLoopAnimation)? std::fmod(animation.mAccumulatedTime + delta, animation.mLength) :
			animation.mAccumulatedTime + delta;
		animation.mAccumulatedTime += delta;

		// Calculate the interpolated KeyFrame between the previous and the current one
		auto [keyFrame1, keyFrame2] = getPreviousAndNextKeyFrames(animation.mKeyFrames, timeSinceStart);
		float keyFrameLength = keyFrame2.timePoint - keyFrame1.timePoint;
		float timeSinceFirstKeyFrame = timeSinceStart - keyFrame1.timePoint;

		KeyFrame keyFrameToApply = keyFrame1;
		if (keyFrameLength > 0.0f) {
			keyFrameToApply = keyFrameLinearInterpolation(
				keyFrame1, keyFrame2,
				timeSinceFirstKeyFrame / keyFrameLength
			);
		}

		// Apply the KeyFrame to the bone
		animation.mCurrentBone.scale = animation.mInitialBone.scale * keyFrameToApply.scale;
		animation.mCurrentBone.orientation = animation.mInitialBone.orientation * keyFrameToApply.rotation;
		animation.mCurrentBone.position = animation.mInitialBone.position + keyFrameToApply.translation;
	}


	std::pair<KeyFrame, KeyFrame> AnimationSystem::getPreviousAndNextKeyFrames(
		const std::vector<KeyFrame>& keyFrames, float timePoint
	) {
		std::pair<KeyFrame, KeyFrame> ret;

		if (!keyFrames.empty()) {
			auto itKeyFrame = std::upper_bound(
				keyFrames.begin(), keyFrames.end(), timePoint,
				[](float timePoint, const KeyFrame& keyFrame) { return keyFrame.timePoint > timePoint; }
			);
			if (itKeyFrame != keyFrames.end()) {
				ret.second = *itKeyFrame;
				if (itKeyFrame != keyFrames.begin()) {
					--itKeyFrame;
					ret.first = *itKeyFrame;
				}
			}
			else {
				ret.first = ret.second = keyFrames.back();
			}
		}

		return ret;
	}

}
