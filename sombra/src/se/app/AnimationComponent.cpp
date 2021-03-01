#include "se/app/AnimationComponent.h"

namespace se::app {

	AnimationComponent::~AnimationComponent()
	{
		if (mNode) {
			for (auto& animator : mSAnimators) {
				animator->removeNodeHierarchy(*mNode);
			}
		}
	}


	void AnimationComponent::setRootNode(animation::AnimationNode* node)
	{
		if (mNode) {
			for (auto& animator : mSAnimators) {
				animator->removeNodeHierarchy(*mNode);
			}
		}
		mNode = node;
		if (mNode) {
			for (auto& animator : mSAnimators) {
				animator->addNodeHierarchy(*mNode);
			}
		}
	}


	void AnimationComponent::addAnimator(SkeletonAnimatorSPtr sAnimator)
	{
		mSAnimators.push_back(sAnimator);
		if (mNode) {
			sAnimator->addNodeHierarchy(*mNode);
		}
	}


	void AnimationComponent::removeAnimator(SkeletonAnimatorSPtr sAnimator)
	{
		if (mNode) {
			sAnimator->removeNodeHierarchy(*mNode);
		}
		mSAnimators.erase(
			std::remove(mSAnimators.begin(), mSAnimators.end(), sAnimator),
			mSAnimators.end()
		);
	}

}
