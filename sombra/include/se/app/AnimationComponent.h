#ifndef ANIMATION_COMPONENT_H
#define ANIMATION_COMPONENT_H

#include "../animation/SkeletonAnimator.h"
#include "Repository.h"

namespace se::app {

	/**
	 * Class AnimationComponent, it holds the node used for animating a
	 * Hierarchy of Entities/Bones
	 */
	class AnimationComponent
	{
	private:	// Nested types
		using SkeletonAnimatorResource =
			Repository::ResourceRef<animation::SkeletonAnimator>;

	private:	// Attributes
		/** A pointer to the root AnimationNode of the hierarchy */
		animation::AnimationNode* mNode;

		/** All the SkeletonAnimators added to the AnimationComponent */
		std::vector<SkeletonAnimatorResource> mSAnimators;

	public:		// Functions
		/** Creates a new AnimationComponent
		 *
		 * @param	node a pointer to the root AnimationNode of the Hierarchy
		 *			to animate */
		AnimationComponent(animation::AnimationNode* node = nullptr) :
			mNode(node) {};
		AnimationComponent(const AnimationComponent& other) = default;
		AnimationComponent(AnimationComponent&& other) = default;

		/** Class destructor */
		~AnimationComponent()
		{
			if (mNode) {
				for (auto& animator : mSAnimators) {
					animator->removeNodeHierarchy(*mNode);
				}
			}
		};

		/** Assignment operator */
		AnimationComponent& operator=(const AnimationComponent& other) =
			default;
		AnimationComponent& operator=(AnimationComponent&& other) = default;

		/** Sets the root Animation node of the AnimationComponent
		 *
		 * @param	node a pointer to the new root AnimationNode of the
		 *			hierarchy
		 * @return	a reference to the current AnimationComponent object */
		AnimationComponent& setRootNode(animation::AnimationNode* node)
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
			return *this;
		};

		/** @return	a pointer to the root AnimationNode of the hierarchy */
		animation::AnimationNode* getRootNode() const { return mNode; };

		/** Adds the given animator to the AnimationComponent
		 *
		 * @param	sAnimator a pointer to the SkeletonAnimator to add
		 * @return	a reference to the current AnimationComponent object */
		AnimationComponent& addAnimator(SkeletonAnimatorResource sAnimator)
		{
			mSAnimators.push_back(sAnimator);
			if (mNode) {
				sAnimator->addNodeHierarchy(*mNode);
			}
			return *this;
		};

		/** Iterates through all the SkeletonAnimators of the AnimationComponent
		 * calling the given callback function
		 *
		 * @param	callback the function to call for each SkeletonAnimator */
		template <typename F>
		void processSAnimators(F&& callback) const
		{
			for (auto& sAnimator : mSAnimators) {
				callback(sAnimator);
			}
		}

		/** Removes the given animator from the AnimationComponent
		 *
		 * @param	sAnimator a pointer to the SkeletonAnimator to remove
		 * @return	a reference to the current AnimationComponent object */
		AnimationComponent& removeAnimator(SkeletonAnimatorResource sAnimator)
		{
			if (mNode) {
				sAnimator->removeNodeHierarchy(*mNode);
			}
			mSAnimators.erase(
				std::remove(mSAnimators.begin(), mSAnimators.end(), sAnimator),
				mSAnimators.end()
			);
			return *this;
		};
	};

}

#endif		// ANIMATION_COMPONENT_H
