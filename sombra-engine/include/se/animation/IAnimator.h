#ifndef I_ANIMATOR_H
#define I_ANIMATOR_H

#include <vector>
#include "se/animation/AnimationNode.h"

namespace se::animation {

	/**
	 * Class IAnimator, it's used to apply an animation to the AnimationNodes
	 */
	class IAnimator
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
		};

	protected:	// Attributes
		/** The nodes to apply the animation transformations */
		std::vector<AnimatedNode> mNodes;

		/** The elapsed time in seconds since the start of the Animation */
		float mAccumulatedTime;

	public:		// Functions
		/** Creates a new IAnimator */
		IAnimator() : mAccumulatedTime(0.0f) {};

		/** Class destructor */
		virtual ~IAnimator() {};

		/** Adds a Node to animate
		 *
		 * @param	type the type of transformation to apply to the node
		 * @param	node a pointer to the AnimatioNode to apply the
		 *			transforms */
		void addNode(TransformationType type, AnimationNode* node)
		{ mNodes.push_back({ type, node }); };

		/** Applies the animation to the nodes of the Animator
		 *
		 * @param	elapsedTime the time elapsed since the last call to the
		 *			function
		 * @note	the world space transformation of the nodes won't be
		 *			updated. */
		virtual void animate(float elapsedTime) = 0;

		/** Restarts the animation to its original state */
		void restartAnimation();

		/** Resets the animate state of every added node */
		void resetNodesAnimatedState();

		/** Updates the added nodes world transforms (and their descendants)
		 * with the changes made by the animation */
		void updateNodesWorldTransforms();
	};

}

#endif		// I_ANIMATOR_H
