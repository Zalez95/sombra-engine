#ifndef TRANSFORMATION_ANIMATOR_H
#define TRANSFORMATION_ANIMATOR_H

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
		float mLoopTime = 0.0f;

		/** The nodes to apply the animation transformations */
		std::vector<AnimatedNode> mNodes;

	public:		// Functions
		/** Class destructor */
		virtual ~TransformationAnimator() = default;

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

		/** Adds a Node to animate
		 *
		 * @param	type the type of transformation to apply to the node
		 * @param	node the AnimatioNode to apply the transforms */
		void addNode(TransformationType type, AnimationNode& node);

		/** Rewinds the Animation applied to the given AnimationNode to the
		 * start, so the next time we call @see animate the AnimationNode
		 * will move like the first time
		 *
		 * @param	type the type of transformation to rewind
		 * @param	node the AnimationNode to rewind its Animation */
		void rewindNode(TransformationType type, AnimationNode& node);

		/** Removes a Node from the IAnimator
		 *
		 * @param	type the type of transformation to remove
		 * @param	node the AnimatioNode to remove from the IAnimator */
		void removeNode(TransformationType type, AnimationNode& node);
	protected:
		/** Animates the given AnimatedNode
		 *
		 * @param	aNode the AnimatedNode to update
		 * @param	elapsedTime the elpased time in seconds since the last
		 *			update */
		virtual void animateNode(AnimatedNode& aNode, float elapsedTime) = 0;
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

		/** @return	a pointer to the Vec3Animation of the Vec3Animator */
		Vec3AnimationSPtr getAnimation() const { return mAnimation; };

		/** Class destructor */
		virtual ~Vec3Animator();
	protected:
		/** @copydoc TransformationAnimator::animateNode(AnimatedNode&, float)
		 * @note	the animation will be applied only to the AnimationNodes
		 *			added with a Translation or Scale transformation */
		virtual void animateNode(
			AnimatedNode& aNode, float elapsedTime
		) override;
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

		/** @return	a pointer to the QuatAnimation of the QuatAnimator */
		QuatAnimationSPtr getAnimation() const { return mAnimation; };

		/** Class destructor */
		virtual ~QuatAnimator();
	protected:
		/** @copydoc TransformationAnimator::animateNode(AnimatedNode&, float)
		 * @note	the animation will be applied only to the AnimationNodes
		 *			added with a Rotation transformation */
		virtual void animateNode(
			AnimatedNode& aNode, float elapsedTime
		) override;
	};

}

#endif		// TRANSFORMATION_ANIMATOR_H
