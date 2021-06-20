#ifndef I_ANIMATOR_H
#define I_ANIMATOR_H

#include <memory>
#include <functional>
#include "AnimationNode.h"

namespace se::animation {

	/**
	 * Class IAnimator, it's used to apply animations to AnimationNodes
	 */
	class IAnimator
	{
	public:		// Nested types
		using AnimationNodeCallback =
			std::function<void(IAnimator&, AnimationNode&)>;

	public:		// Functions
		/** Class destructor */
		virtual ~IAnimator() = default;

		/** @return	a pointer to a copy of the current IAnimator */
		virtual std::unique_ptr<IAnimator> clone() const = 0;

		/** @return	the elapsed time in seconds since the start of the animation
		 *			before the animation starts from the begining */
		virtual float getLoopTime() const = 0;

		/** Sets the loop time of the IAnimator
		 *
		 * @param	loopTime the time in seconds since the start of the
		 *			animation before it starts again */
		virtual void setLoopTime(float loopTime) = 0;

		/** Applies the animation to the nodes of the IAnimator
		 *
		 * @param	elapsedTime the time elapsed since the last call to the
		 *			function
		 * @note	the world space transformation of the nodes won't be
		 *			updated. */
		virtual void animate(float elapsedTime) = 0;

		/** Resets the animated state of every node */
		virtual void resetNodesAnimatedState() = 0;

		/** Updates the IAnimator's nodes (and their descendants) with the
		 * changes made by the animations */
		virtual void updateNodesHierarchy() = 0;
	};

}

#endif		// I_ANIMATOR_H
