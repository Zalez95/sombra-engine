#include <algorithm>
#include "se/animation/AnimationSystem.h"

namespace se::animation {

	void AnimationSystem::addAnimator(IAnimator* animator)
	{
		if (animator) {
			mAnimators.push_back(animator);
		}
	}


	void AnimationSystem::removeAnimator(IAnimator* animator)
	{
		mAnimators.erase(
			std::remove(mAnimators.begin(), mAnimators.end(), animator),
			mAnimators.end()
		);
	}


	void AnimationSystem::update(float deltaTime)
	{
		for (IAnimator* animator : mAnimators) {
			animator->animate(deltaTime);
		}
	}

}
