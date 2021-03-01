#include "se/animation/TransformationAnimator.h"

namespace se::animation {

	float TransformationAnimator::getLoopTime() const
	{
		return mLoopTime;
	}


	void TransformationAnimator::setLoopTime(float loopTime)
	{
		mLoopTime = loopTime;
	}


	void TransformationAnimator::animate(float elapsedTime)
	{
		for (auto& animatedNode : mNodes) {
			animateNode(animatedNode, elapsedTime);
		}
	}


	void TransformationAnimator::resetNodesAnimatedState()
	{
		for (AnimatedNode& animatedNode : mNodes) {
			if (animatedNode.node->getData().animated) {
				updateAnimatedState(*animatedNode.node, false);
			}
		}
	}


	void TransformationAnimator::updateNodesHierarchy()
	{
		for (AnimatedNode& animatedNode : mNodes) {
			if (!animatedNode.node->getData().worldTransformsUpdated) {
				updateWorldTransforms(*animatedNode.node);
				updateAnimatedState(*animatedNode.node, true);
			}
		}
	}


	void TransformationAnimator::addNode(TransformationType type, AnimationNode& node)
	{
		mNodes.push_back({ type, &node, 0.0f });
	}


	void TransformationAnimator::rewindNode(TransformationType type, AnimationNode& node)
	{
		auto it = std::find_if(mNodes.begin(), mNodes.end(), [&](const AnimatedNode& aNode) {
			return (aNode.type == type) && (aNode.node == &node);
		});
		if (it != mNodes.end()) {
			it->accumulatedTime = 0.0f;
			animateNode(*it, 0.0f);
		}
	}


	void TransformationAnimator::removeNode(TransformationType type, AnimationNode& node)
	{
		auto it = std::find_if(mNodes.begin(), mNodes.end(), [&](const AnimatedNode& aNode) {
			return (aNode.type == type) && (aNode.node == &node);
		});
		if (it != mNodes.end()) {
			it->accumulatedTime = 0.0f;
			animateNode(*it, 0.0f);
			mNodes.erase(it);
		}
	}


	Vec3Animator::Vec3Animator(Vec3AnimationSPtr animation) : mAnimation(animation)
	{
		setLoopTime(mAnimation->getLength());
	}


	Vec3Animator::~Vec3Animator()
	{
		for (auto& animatedNode : mNodes) {
			rewindNode(animatedNode.type, *animatedNode.node);
		}
	}


	void Vec3Animator::animateNode(AnimatedNode& aNode, float elapsedTime)
	{
		aNode.accumulatedTime = std::fmod(aNode.accumulatedTime + elapsedTime, getLoopTime());
		glm::vec3 transformation = mAnimation->interpolate(aNode.accumulatedTime);

		switch (aNode.type) {
			case TransformationType::Translation:
				aNode.node->getData().localTransforms.position = transformation;
				aNode.node->getData().worldTransformsUpdated = false;
				break;
			case TransformationType::Scale:
				aNode.node->getData().localTransforms.scale = transformation;
				aNode.node->getData().worldTransformsUpdated = false;
				break;
			default:
				break;
		}
	}


	QuatAnimator::QuatAnimator(QuatAnimationSPtr animation) : mAnimation(animation)
	{
		setLoopTime(mAnimation->getLength());
	}


	QuatAnimator::~QuatAnimator()
	{
		for (auto& animatedNode : mNodes) {
			rewindNode(animatedNode.type, *animatedNode.node);
		}
	}


	void QuatAnimator::animateNode(AnimatedNode& aNode, float elapsedTime)
	{
		aNode.accumulatedTime = std::fmod(aNode.accumulatedTime + elapsedTime, getLoopTime());
		glm::quat transformation = mAnimation->interpolate(aNode.accumulatedTime);

		switch (aNode.type) {
			case TransformationType::Rotation:
				aNode.node->getData().localTransforms.orientation = transformation;
				aNode.node->getData().worldTransformsUpdated = false;
				break;
			default:
				break;
		}
	}

}
