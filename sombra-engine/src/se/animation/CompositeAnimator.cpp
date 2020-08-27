#include <algorithm>
#include "se/animation/CompositeAnimator.h"

namespace se::animation {

	float CompositeAnimator::getLoopTime() const
	{
		return mLoopTime;
	}


	void CompositeAnimator::setLoopTime(float loopTime)
	{
		mLoopTime = loopTime;
		for (auto& animator : mAnimators) {
			animator->setLoopTime(mLoopTime);
		}
	}


	void CompositeAnimator::resetNodesAnimatedState()
	{
		for (auto& animator : mAnimators) {
			animator->resetNodesAnimatedState();
		}
	}


	void CompositeAnimator::updateNodesWorldTransforms()
	{
		for (auto& animator : mAnimators) {
			animator->updateNodesWorldTransforms();
		}
	}


	void CompositeAnimator::animate(float elapsedTime)
	{
		for (auto& animator : mAnimators) {
			animator->animate(elapsedTime);
		}
	}


	void CompositeAnimator::addAnimator(IAnimatorUPtr animator)
	{
		animator->setLoopTime(mLoopTime);
		mAnimators.emplace_back(std::move(animator));
	}

}
