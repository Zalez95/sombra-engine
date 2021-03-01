#ifndef SKELETON_ANIMATOR_H
#define SKELETON_ANIMATOR_H

#include "TransformationAnimator.h"

namespace se::animation {

	/**
	 * Class IAnimator, it's used to apply animations to AnimationNode
	 * hierarchies
	 */
	class SkeletonAnimator : public IAnimator
	{
	private:	// Nested types
		using TransformationAnimatorUPtr =
			std::unique_ptr<TransformationAnimator>;

		struct NodeAnimator
		{
			std::array<char, NodeData::kMaxLength> nodeName;
			TransformationAnimator::TransformationType type;
			TransformationAnimatorUPtr animator;
		};

	private:	// Attributes
		/** The elapsed time in seconds since the start of the animation
		 * before all the IAnimators' animations start from the begining */
		float mLoopTime;

		/** Maps the names of the AnimationNodes to transform with the
		 * IAnimators that will animate them */
		std::vector<NodeAnimator> mNAnimators;

		/** All the root AnimationNodes of the hierarchies to animate */
		std::vector<AnimationNode*> mRootNodes;

	public:		// Functions
		/** Creates a new SkeletonAnimator
		 *
		 * @param	loopTime the loop time of the SkeletonAnimator, 0 by
		 *			default */
		SkeletonAnimator(float loopTime = 0.0f) : mLoopTime(loopTime) {};

		/** Class destructor */
		virtual ~SkeletonAnimator();

		/** @copydoc IAnimator::getLoopTime() */
		virtual float getLoopTime() const;

		/** @copydoc IAnimator::setLoopTime(float) */
		virtual void setLoopTime(float loopTime);

		/** @copydoc IAnimator::animate(float) */
		virtual void animate(float elapsedTime) override;

		/** @copydoc IAnimator::resetNodesAnimatedState() */
		virtual void resetNodesAnimatedState() override;

		/** @copydoc IAnimator::updateNodesHierarchy() */
		virtual void updateNodesHierarchy() override;

		/** Adds the given TransformationAnimator to the SkeletonAnimator
		 *
		 * @param	nodeName the name of the AnimationNodes that will be
		 *			affected by the animator
		 * @param	type the TransformationType to apply to the AnimationNode
		 *			with the given TransformationAnimator
		 * @param	animator a pointer to the TransformationAnimator to add */
		void addAnimator(
			const std::array<char, NodeData::kMaxLength>& nodeName,
			TransformationAnimator::TransformationType type,
			TransformationAnimatorUPtr animator
		);

		/** Adds the given node hierarchy to the SkeletonAnimator, so the nodes
		 * will be animated when the IAnimator is updated
		 *
		 * @param	rootNode the root AnimationNode of the skeleton to
		 *			animate */
		void addNodeHierarchy(AnimationNode& rootNode);

		/** Rewinds the Animation applied to the given AnimationNode hierarchy
		 * to the start, so the next time we call @see animate the hierarchy
		 * will move like the first time
		 *
		 * @param	node the root AnimationNode to rewind its Animation */
		void rewindNodeHierarchy(AnimationNode& rootNode);

		/** Removes the given node hierarchy from the SkeletonAnimator, so the
		 * nodes will no longer be animated when the IAnimator is updated
		 *
		 * @param	rootNode the root AnimationNode of the skeleton to remove */
		void removeNodeHierarchy(AnimationNode& rootNode);
	private:
		/** Compare the names of the animator and the node
		 *
		 * @param	lhs the NodeAnimator to compare
		 * @param	rhs the AnimationNode to compare
		 * @return	true if the NodeAnimator is less than the AnimationNode,
		 *			false otherwise */
		static
		bool compareLessLo(const NodeAnimator& lhs, const AnimationNode& rhs);

		/** Compare the names of the node and the animator
		 *
		 * @param	lhs the AnimationNode to compare
		 * @param	rhs the NodeAnimator to compare
		 * @return	true if the AnimationNode is less than the NodeAnimator,
		 *			false otherwise */
		static
		bool compareLessUp(const AnimationNode& lhs, const NodeAnimator& rhs);
	};

}

#endif		// SKELETON_ANIMATOR_H
