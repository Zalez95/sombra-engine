#ifndef ANIMATION_ENGINE_H
#define ANIMATION_ENGINE_H

#include <mutex>
#include <vector>
#include "IAnimator.h"

namespace se::animation {

	/**
	 * Class AnimationEngine, it's used to update the Animators
	 */
	class AnimationEngine
	{
	private:	// Attributes
		/** The Animators to update */
		std::vector<IAnimator*> mAnimators;

		/** The mutex used for protecting @see mAnimators */
		std::mutex mMutex;

	public:		// Functions
		/** Adds the given IAnimator to the AnimationEngine so it will be
		 * updated in each @ref update(float) call
		 *
		 * @param	animator a pointer to the IAnimator to add */
		void addAnimator(IAnimator* animator);

		/** Removes the given IAnimator from the AnimationEngine so it won't
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

#endif		// ANIMATION_ENGINE_H
