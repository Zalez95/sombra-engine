#include <cstring>
#include <algorithm>
#include "se/animation/SkeletonAnimator.h"

namespace se::animation {

	SkeletonAnimator::~SkeletonAnimator()
	{
		while (!mRootNodes.empty()) {
			removeNodeHierarchy(*mRootNodes.back());
		}
	}


	float SkeletonAnimator::getLoopTime() const
	{
		return mLoopTime;
	}


	void SkeletonAnimator::setLoopTime(float loopTime)
	{
		mLoopTime = loopTime;
		for (auto& nAnimator : mNAnimators) {
			nAnimator.animator->setLoopTime(mLoopTime);
		}
	}


	void SkeletonAnimator::animate(float elapsedTime)
	{
		for (auto& nAnimator : mNAnimators) {
			nAnimator.animator->animate(elapsedTime);
		}
	}


	void SkeletonAnimator::resetNodesAnimatedState()
	{
		for (AnimationNode* root : mRootNodes) {
			updateAnimatedState(*root, false);
		}
	}


	void SkeletonAnimator::updateNodesHierarchy()
	{
		for (AnimationNode* root : mRootNodes) {
			if (!root->getData().animated) {
				updateWorldTransforms(*root);
				updateAnimatedState(*root, true);
			}
		}
	}


	void SkeletonAnimator::addAnimator(
		const std::array<char, NodeData::kMaxLength>& nodeName,
		TransformationAnimator::TransformationType type,
		TransformationAnimatorUPtr animator
	) {
		animator->setLoopTime(mLoopTime);

		NodeAnimator nodeAnimator = { nodeName, type, std::move(animator) };
		auto it = std::lower_bound(
			mNAnimators.begin(), mNAnimators.end(), nodeAnimator,
			[](const NodeAnimator &lhs, const NodeAnimator &rhs) {
				return std::strcmp(lhs.nodeName.data(), rhs.nodeName.data()) < 0;
			}
		);
		mNAnimators.emplace(it, std::move(nodeAnimator));
	}


	void SkeletonAnimator::addNodeHierarchy(AnimationNode& rootNode)
	{
		mRootNodes.push_back(&rootNode);
		for (auto& descendant : rootNode) {
			auto itL = std::lower_bound(mNAnimators.begin(), mNAnimators.end(), descendant, compareLessLo);
			auto itU = std::upper_bound(mNAnimators.begin(), mNAnimators.end(), descendant, compareLessUp);
			for (auto it = itL; it != itU; ++it) {
				it->animator->addNode(it->type, descendant);
			}
		}
	}


	void SkeletonAnimator::rewindNodeHierarchy(AnimationNode& rootNode)
	{
		for (auto& descendant : rootNode) {
			auto itL = std::lower_bound(mNAnimators.begin(), mNAnimators.end(), descendant, compareLessLo);
			auto itU = std::upper_bound(mNAnimators.begin(), mNAnimators.end(), descendant, compareLessUp);
			for (auto it = itL; it != itU; ++it) {
				it->animator->rewindNode(it->type, descendant);
			}
		}
	}


	void SkeletonAnimator::removeNodeHierarchy(AnimationNode& rootNode)
	{
		mRootNodes.erase(
			std::remove(mRootNodes.begin(), mRootNodes.end(), &rootNode),
			mRootNodes.end()
		);
		for (auto& descendant : rootNode) {
			auto itL = std::lower_bound(mNAnimators.begin(), mNAnimators.end(), descendant, compareLessLo);
			auto itU = std::upper_bound(mNAnimators.begin(), mNAnimators.end(), descendant, compareLessUp);
			for (auto it = itL; it != itU; ++it) {
				it->animator->removeNode(it->type, descendant);
			}
		}
	}

// Private function
	bool SkeletonAnimator::compareLessLo(const NodeAnimator& lhs, const AnimationNode& rhs)
	{
		return std::strcmp(lhs.nodeName.data(), rhs.getData().name.data()) < 0;
	}


	bool SkeletonAnimator::compareLessUp(const AnimationNode& lhs, const NodeAnimator& rhs)
	{
		return std::strcmp(lhs.getData().name.data(), rhs.nodeName.data()) < 0;
	}

}
