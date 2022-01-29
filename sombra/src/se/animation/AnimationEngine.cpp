#include <algorithm>
#include "se/animation/AnimationEngine.h"

namespace se::animation {

	void AnimationEngine::addAnimator(IAnimator* animator)
	{
		if (!animator) { return; }

		std::scoped_lock lck(mMutex);
		mAnimators.push_back(animator);
	}


	void AnimationEngine::removeAnimator(IAnimator* animator)
	{
		if (!animator) { return; }

		animator->resetNodesAnimatedState();

		std::scoped_lock lck(mMutex);
		mAnimators.erase(
			std::remove(mAnimators.begin(), mAnimators.end(), animator),
			mAnimators.end()
		);
	}


	void AnimationEngine::update(float deltaTime)
	{
		std::scoped_lock lck(mMutex);

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
			animator->updateNodesHierarchy();
		}
	}

}
