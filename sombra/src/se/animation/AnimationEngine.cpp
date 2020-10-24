#include <algorithm>
#include "se/animation/AnimationEngine.h"

namespace se::animation {

	void AnimationEngine::addAnimator(IAnimator* animator)
	{
		if (!animator) { return; }

		mAnimators.push_back(animator);
	}


	void AnimationEngine::removeAnimator(IAnimator* animator)
	{
		if (!animator) { return; }

		animator->resetNodesAnimatedState();

		mAnimators.erase(
			std::remove(mAnimators.begin(), mAnimators.end(), animator),
			mAnimators.end()
		);
	}


	void AnimationEngine::update(float deltaTime)
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
