#include "se/animation/TransformationAnimators.h"

namespace se::animation {

	float TransformationAnimator::getLoopTime() const
	{
		return mLoopTime;
	}


	void TransformationAnimator::setLoopTime(float loopTime)
	{
		mLoopTime = loopTime;
	}


	void TransformationAnimator::resetNodesAnimatedState()
	{
		for (AnimatedNode& animatedNode : mNodes) {
			animatedNode.node->getData().animated = false;
		}
	}


	void TransformationAnimator::updateNodesWorldTransforms()
	{
		for (AnimatedNode& animatedNode : mNodes) {
			if (!animatedNode.node->getData().worldTransformsUpdated) {
				updateWorldTransforms(*animatedNode.node);
			}
		}
	}


	void TransformationAnimator::addNode(TransformationType type, AnimationNode* node)
	{
		mNodes.push_back({ type, node, 0.0f });
	}


	void TransformationAnimator::removeNode(AnimationNode* node)
	{
		mNodes.erase(
			std::remove_if(mNodes.begin(), mNodes.end(), [&](const AnimatedNode& aNode) {
				return aNode.node == node;
			}),
			mNodes.end()
		);
	}


	Vec3Animator::Vec3Animator(Vec3AnimationSPtr animation) : mAnimation(animation)
	{
		setLoopTime(mAnimation->getLength());
	}


	void Vec3Animator::animate(float elapsedTime)
	{
		for (auto& animationNode : mNodes) {
			animationNode.accumulatedTime = std::fmod(animationNode.accumulatedTime + elapsedTime, getLoopTime());
			glm::vec3 transformation = mAnimation->interpolate(animationNode.accumulatedTime);

			switch (animationNode.type) {
				case TransformationType::Translation:
					animationNode.node->getData().localTransforms.position = transformation;
					animationNode.node->getData().animated = true;
					animationNode.node->getData().worldTransformsUpdated = false;
					break;
				case TransformationType::Scale:
					animationNode.node->getData().localTransforms.scale = transformation;
					animationNode.node->getData().animated = true;
					animationNode.node->getData().worldTransformsUpdated = false;
					break;
				default:
					break;
			}
		}
	}


	QuatAnimator::QuatAnimator(QuatAnimationSPtr animation) : mAnimation(animation)
	{
		setLoopTime(mAnimation->getLength());
	}


	void QuatAnimator::animate(float elapsedTime)
	{
		for (auto& animationNode : mNodes) {
			animationNode.accumulatedTime = std::fmod(animationNode.accumulatedTime + elapsedTime, getLoopTime());
			glm::quat transformation = mAnimation->interpolate(animationNode.accumulatedTime);

			switch (animationNode.type) {
				case TransformationType::Rotation:
					animationNode.node->getData().localTransforms.orientation = transformation;
					animationNode.node->getData().animated = true;
					animationNode.node->getData().worldTransformsUpdated = false;
					break;
				default:
					break;
			}
		}
	}

}
