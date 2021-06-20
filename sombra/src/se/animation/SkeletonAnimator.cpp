#include <cstring>
#include <algorithm>
#include "se/animation/SkeletonAnimator.h"

namespace se::animation {

	SkeletonAnimator::SkeletonAnimator(const SkeletonAnimator& other)
	{
		mLoopTime = other.mLoopTime;
		mRootNodes = other.mRootNodes;
		for (const NodeAnimator& nodeAnimator : other.mNAnimators) {
			auto transformsAnimator = dynamic_cast<TransformationAnimator*>(nodeAnimator.animator->clone().release());

			NodeAnimator nodeAnimator2;
			nodeAnimator2.nodeName = nodeAnimator.nodeName;
			nodeAnimator2.type = nodeAnimator.type;
			nodeAnimator2.animator = std::unique_ptr<TransformationAnimator>(transformsAnimator);
			mNAnimators.emplace_back(std::move(nodeAnimator2));
		}
	}


	SkeletonAnimator::~SkeletonAnimator()
	{
		while (!mRootNodes.empty()) {
			removeNodeHierarchy(*mRootNodes.back());
		}
	}


	SkeletonAnimator& SkeletonAnimator::operator=(const SkeletonAnimator& other)
	{
		while (!mRootNodes.empty()) {
			removeNodeHierarchy(*mRootNodes.back());
		}

		mLoopTime = other.mLoopTime;
		mRootNodes = other.mRootNodes;
		for (const NodeAnimator& nodeAnimator : other.mNAnimators) {
			auto transformsAnimator = dynamic_cast<TransformationAnimator*>(nodeAnimator.animator->clone().release());

			NodeAnimator nodeAnimator2;
			nodeAnimator2.nodeName = nodeAnimator.nodeName;
			nodeAnimator2.type = nodeAnimator.type;
			nodeAnimator2.animator = std::unique_ptr<TransformationAnimator>(transformsAnimator);
			mNAnimators.emplace_back(std::move(nodeAnimator2));
		}

		return *this;
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
		const char* nodeName, TransformationAnimator::TransformationType type,
		TransformationAnimatorUPtr animator
	) {
		animator->setLoopTime(mLoopTime);

		NodeAnimator nodeAnimator = { {}, type, std::move(animator) };
		std::copy(nodeName, nodeName + std::strlen(nodeName), nodeAnimator.nodeName.begin());
		auto it = std::lower_bound(mNAnimators.begin(), mNAnimators.end(), nodeAnimator.nodeName.data(), compareLessLo);
		mNAnimators.emplace(it, std::move(nodeAnimator));
	}


	void SkeletonAnimator::removeAnimator(const char* nodeName, TransformationAnimator::TransformationType type)
	{
		auto itL = std::lower_bound(mNAnimators.begin(), mNAnimators.end(), nodeName, compareLessLo);
		auto itU = std::upper_bound(mNAnimators.begin(), mNAnimators.end(), nodeName, compareLessUp);
		mNAnimators.erase(
			std::remove_if(itL, itU, [&](const NodeAnimator& nodeAnimator) { return nodeAnimator.type == type; }),
			itU
		);
	}


	void SkeletonAnimator::addNodeHierarchy(AnimationNode& rootNode)
	{
		mRootNodes.push_back(&rootNode);
		for (auto& descendant : rootNode) {
			auto itL = std::lower_bound(mNAnimators.begin(), mNAnimators.end(), descendant.getData().name.data(), compareLessLo);
			auto itU = std::upper_bound(mNAnimators.begin(), mNAnimators.end(), descendant.getData().name.data(), compareLessUp);
			for (auto it = itL; it != itU; ++it) {
				it->animator->addNode(it->type, descendant);
			}
		}
	}


	void SkeletonAnimator::rewindNodeHierarchy(AnimationNode& rootNode)
	{
		for (auto& descendant : rootNode) {
			auto itL = std::lower_bound(mNAnimators.begin(), mNAnimators.end(), descendant.getData().name.data(), compareLessLo);
			auto itU = std::upper_bound(mNAnimators.begin(), mNAnimators.end(), descendant.getData().name.data(), compareLessUp);
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
			auto itL = std::lower_bound(mNAnimators.begin(), mNAnimators.end(), descendant.getData().name.data(), compareLessLo);
			auto itU = std::upper_bound(mNAnimators.begin(), mNAnimators.end(), descendant.getData().name.data(), compareLessUp);
			for (auto it = itL; it != itU; ++it) {
				it->animator->removeNode(it->type, descendant);
			}
		}
	}

// Private function
	bool SkeletonAnimator::compareLessLo(const NodeAnimator& lhs, const char* rhs)
	{
		return std::strcmp(lhs.nodeName.data(), rhs) < 0;
	}


	bool SkeletonAnimator::compareLessUp(const char* lhs, const NodeAnimator& rhs)
	{
		return std::strcmp(lhs, rhs.nodeName.data()) < 0;
	}

}
