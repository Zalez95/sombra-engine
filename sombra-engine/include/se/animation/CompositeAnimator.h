#ifndef COMPOSITE_ANIMATOR_H
#define COMPOSITE_ANIMATOR_H

#include <memory>
#include <vector>
#include "IAnimator.h"

namespace se::animation {

	/**
	 * Class CompositeAnimator, it's used to animate a set of IAnimators at the
	 * same time
	 */
	class CompositeAnimator : public IAnimator
	{
	private:	// Nested types
		using IAnimatorUPtr = std::unique_ptr<IAnimator>;

	private:	// Attributes
		/** The IAnimators of the CompositeAnimator */
		std::vector<IAnimatorUPtr> mAnimators;

		/**	The elapsed time in seconds since the start of the animation
		 * before all the IAnimators' animations start from the begining  */
		float mLoopTime;

	public:		// Functions
		/** Creates a new CompositeAnimator
		 *
		 * @param	loopTime the loop time of the CompositeAnimator, 0 by
		 *			default */
		CompositeAnimator(float loopTime = 0.0f) : mLoopTime(loopTime) {};

		/** @return	the elapsed time in seconds since the start of the animation
		 *			before all the IAnimators' animations start from the
		 *			begining */
		virtual float getLoopTime() const override;

		/** Sets the loop time of all the IAnimators
		 *
		 * @param	loopTime the new time in seconds since the start of the
		 *			animation before all of the IAnimators' animations start
		 *			again */
		virtual void setLoopTime(float loopTime) override;

		/** Animates all the IAnimators of the CompositeAnimator
		 *
		 * @param	elapsedTime the time elapsed since the last call to the
		 *			function */
		virtual void animate(float elapsedTime) override;

		/** Resets the animate state of every IAnimators' node */
		virtual void resetNodesAnimatedState() override;

		/** Updates the IAnimators' nodes world transforms (and their
		 * descendants) with the changes made by their animation */
		virtual void updateNodesWorldTransforms() override;

		/** Adds the given IAnimator to the Composite
		 *
		 * @param	animator the IAnimator to add
		 * @note	the loopTime of the new IAnimators will be changed to the
		 *			CompositeAnimator one */
		void addAnimator(IAnimatorUPtr animator);
	};

}

#endif		// COMPOSITE_ANIMATOR_H
