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
		animator->resetNodesAnimatedState();

		mAnimators.erase(
			std::remove(mAnimators.begin(), mAnimators.end(), animator),
			mAnimators.end()
		);
	}


	void AnimationSystem::update(float deltaTime)
	{
		// Reset the animate state of all the nodes
		for (IAnimator* animator : mAnimators) {
			animator->resetNodesAnimatedState();
		}

		// Update the Animations
		for (IAnimator* animator : mAnimators) {
			animator->animate(deltaTime);
		}

		// Update the world transforms of all the nodes
		for (IAnimator* animator : mAnimators) {
			animator->updateNodesWorldTransforms();
		}
	}

}
