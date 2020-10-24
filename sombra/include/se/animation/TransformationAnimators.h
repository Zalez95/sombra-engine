#ifndef TRANSFORMATION_ANIMATORS_H
#define TRANSFORMATION_ANIMATORS_H

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "IAnimator.h"
#include "IAnimation.h"
#include "AnimationNode.h"

namespace se::animation {

	/**
	 * Class TransformationAnimator, it's used to apply a transform animation
	 * to its AnimationNodes
	 */
	class TransformationAnimator : public IAnimator
	{
	public:		// Nested types
		/** The type of transformation to apply to a AnimationNode */
		enum class TransformationType
		{
			Translation, Rotation, Scale
		};
	protected:
		/** Maps the AnimationNodes with the type of transformation to apply to
		 * them */
		struct AnimatedNode
		{
			/** The type of transformation to apply to the node */
			TransformationType type;

			/** The node to apply the transforms */
			AnimationNode* node;

			/** The elapsed time in seconds since the start of the Animation */
			float accumulatedTime;
		};

	protected:	// Attributes
		/** The elapsed time in seconds since the start of the animation before
		 * it starts from the begining */
		float mLoopTime;

		/** The nodes to apply the animation transformations */
		std::vector<AnimatedNode> mNodes;

	public:		// Functions
		/** Creates a new TransformationAnimator */
		TransformationAnimator() : mLoopTime(0.0f) {};

		/** Class destructor */
		virtual ~TransformationAnimator() = default;

		/** @return	the elapsed time in seconds since the start of the animation
		 *			before the animation starts from the begining */
		virtual float getLoopTime() const;

		/** Sets the loop time of the IAnimator
		 *
		 * @param	loopTime the time in seconds since the start of the
		 *			animation before it starts again */
		virtual void setLoopTime(float loopTime);

		/** Applies the animation to the nodes of the Animator
		 *
		 * @param	elapsedTime the time elapsed since the last call to the
		 *			function
		 * @note	the world space transformation of the nodes won't be
		 *			updated. */
		virtual void animate(float elapsedTime) = 0;

		/** Resets the animate state of every added node */
		virtual void resetNodesAnimatedState() override;

		/** Updates the added nodes world transforms (and their descendants)
		 * with the changes made by the animation */
		virtual void updateNodesWorldTransforms() override;

		/** Adds a Node to animate
		 *
		 * @param	type the type of transformation to apply to the node
		 * @param	node a pointer to the AnimatioNode to apply the
		 *			transforms */
		void addNode(TransformationType type, AnimationNode* node);

		/** Iterates through all the Nodes of the TransformationAnimator
		 * calling the given callback function
		 *
		 * @param	callback the function to call for each AnimationNode */
		template <typename F>
		void processNodes(F callback);

		/** Removes a Node from the TransformationAnimator
		 *
		 * @param	node a pointer to the AnimatioNode to remove from the
		 *			TransformationAnimator */
		void removeNode(AnimationNode* node);
	};


	/**
	 * Class Vec3Animator, it's a TransformationAnimator used to apply vec3
	 * transformations to its AnimationNodes
	 */
	class Vec3Animator : public TransformationAnimator
	{
	private:	// Nested types
		using Vec3AnimationSPtr = std::shared_ptr<IAnimation<glm::vec3>>;

	private:	// Attributes
		/** The animation of the Animator */
		Vec3AnimationSPtr mAnimation;

	public:		// Functions
		/** Creates a new Vec3Animator
		 *
		 * @param	animation a pointer to the animation of the Vec3Animator
		 * @note	the initial loopTime of the Vec3Animator is always the
		 *			length of the given Animation */
		Vec3Animator(Vec3AnimationSPtr animation);

		/** Applies the animation to the nodes of the Animator
		 *
		 * @param	elapsedTime the time elapsed since the last call to the
		 *			function
		 * @note	the animation will be applied only to the AnimationNodes
		 *			added with a Translation or Scale transformation */
		virtual void animate(float elapsedTime) override;
	};


	/**
	 * Class QuatAnimator, it's a TransformationAnimator used to apply quat
	 * transformations to its AnimationNodes
	 */
	class QuatAnimator : public TransformationAnimator
	{
	private:	// Nested types
		using QuatAnimationSPtr = std::shared_ptr<IAnimation<glm::quat>>;

	private:	// Attributes
		/** The animation of the Animator */
		QuatAnimationSPtr mAnimation;

	public:		// Functions
		/** Creates a new QuatAnimator
		 *
		 * @param	animation a pointer to the animation of the QuatAnimator
		 * @note	the initial loopTime of the QuatAnimator is always the
		 *			length of the given Animation */
		QuatAnimator(QuatAnimationSPtr animation);

		/** Applies the animation to the nodes of the Animator
		 *
		 * @param	elapsedTime the time elapsed since the last call to the
		 *			function
		 * @note	the animation will be applied only to the AnimationNodes
		 *			added with a Rotation transformation */
		virtual void animate(float elapsedTime) override;
	};


	template <typename F>
	void TransformationAnimator::processNodes(F callback)
	{
		for (AnimatedNode& animatedNode : mNodes) {
			callback(animatedNode.node);
		}
	}

}

#endif		// TRANSFORMATION_ANIMATORS_H
