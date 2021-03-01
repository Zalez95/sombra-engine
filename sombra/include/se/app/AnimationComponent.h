#ifndef ANIMATION_COMPONENT_H
#define ANIMATION_COMPONENT_H

#include "../animation/SkeletonAnimator.h"

namespace se::app {

	/**
	 * Class AnimationComponent, it holds the node used for animating a
	 * Hierarchy of Entities/Bones
	 */
	class AnimationComponent
	{
	private:	// Nested types
		using SkeletonAnimatorSPtr =
			std::shared_ptr<animation::SkeletonAnimator>;

	private:	// Attributes
		/** A pointer to the root AnimationNode of the hierarchy */
		animation::AnimationNode* mNode;

		/** All the SkeletonAnimators added to the AnimationComponent */
		std::vector<SkeletonAnimatorSPtr> mSAnimators;

	public:		// Functions
		/** Creates a new AnimationComponent
		 *
		 * @param	node a pointer to the root AnimationNode of the Hierarchy
		 *			to animate */
		AnimationComponent(animation::AnimationNode* node = nullptr) :
			mNode(node) {};

		/** Class destructor */
		~AnimationComponent();

		/** Sets the root Animation node of the AnimationComponent
		 *
		 * @param	a pointer to the new root AnimationNode of the hierarchy */
		void setRootNode(animation::AnimationNode* node);

		/** @return	a pointer to the root AnimationNode of the hierarchy */
		animation::AnimationNode* getRootNode() const { return mNode; };

		/** Adds the given animator to the AnimationComponent
		 *
		 * @param	sAnimator a pointer to the SkeletonAnimator to add */
		void addAnimator(SkeletonAnimatorSPtr sAnimator);

		/** Iterates through all the SkeletonAnimators of the AnimationComponent
		 * calling the given callback function
		 *
		 * @param	callback the function to call for each SkeletonAnimator */
		template <typename F>
		void processSAnimators(F callback) const;

		/** Removes the given animator from the AnimationComponent
		 *
		 * @param	sAnimator a pointer to the SkeletonAnimator to remove */
		void removeAnimator(SkeletonAnimatorSPtr sAnimator);
	};


	template <typename F>
	void AnimationComponent::processSAnimators(F callback) const
	{
		for (auto& sAnimator : mSAnimators) {
			callback(sAnimator);
		}
	}

}

#endif		// ANIMATION_COMPONENT_H
