#include "se/animation/IAnimator.h"

namespace se::animation {

	void IAnimator::restartAnimation()
	{
		mAccumulatedTime = 0.0f;
	}


	void IAnimator::resetNodesAnimatedState()
	{
		for (AnimatedNode& animatedNode : mNodes) {
			animatedNode.node->getData().animated = false;
		}
	}


	void IAnimator::updateNodesWorldTransforms()
	{
		for (AnimatedNode& animatedNode : mNodes) {
			if (!animatedNode.node->getData().worldTransformsUpdated) {
				updateWorldTransforms(*animatedNode.node);
			}
		}
	}

}
