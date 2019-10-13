#ifndef ANIMATION_SYSTEM_H
#define ANIMATION_SYSTEM_H

#include "IAnimator.h"

namespace se::animation {

	/**
	 * Class AnimationSystem, it's used to update the Animations
	 */
	class AnimationSystem
	{
	private:	// Attributes
		/** The Animators to update */
		std::vector<IAnimator*> mAnimators;

	public:		// Functions
		/** Adds the given IAnimator to the AnimationSystem so it will be
		 * updated in each @ref update(float) call
		 *
		 * @param	animator a pointer to the IAnimator to add */
		void addAnimator(IAnimator* animator);

		/** Removes the given IAnimator from the AnimationSystem so it won't
		 * longer be updated
		 *
		 * @param	animator a pointer to the IAnimator to remove */
		void removeAnimator(IAnimator* animator);

		/** Updates the nodes of each IAnimator with their keyframes and the
		 * given time
		 *
		 * @param	deltaTime the elapsed time since the last update in
		 *			seconds */
		void update(float deltaTime);
	};

}

#endif		// ANIMATION_SYSTEM_H
