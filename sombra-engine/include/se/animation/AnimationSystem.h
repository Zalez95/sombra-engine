#ifndef ANIMATION_SYSTEM_H
#define ANIMATION_SYSTEM_H

#include "Animation.h"

namespace se::animation {

	/**
	 * Class AnimationSystem, it's used to update the Animations
	 */
	class AnimationSystem
	{
	private:	// Attributes
		/** The Animations to update */
		std::vector<Animation*> mAnimations;

	public:		// Functions
		/** Adds the given Animation to the AnimationSystem so it will be
		 * updated in each @ref update(float) call
		 *
		 * @param	animation a pointer to the Animation to remove */
		void addAnimation(Animation* animation);

		/** Removes the given Animation from the AnimationSystem so it won't
		 * longer be updated
		 *
		 * @param	animation a pointer to the Animation to remove */
		void removeAnimation(Animation* animation);

		/** Updates the Skeleton poses of each Animation with their keyframes
		 * and the current time
		 *
		 * @param	delta the elapsed time since the last update in seconds */
		void update(float delta);
	private:
		/** Updates the current Bone state of the given animation by
		 * interpolating its data between the Animation's KeyFrames
		 *
		 * @param	animation the Animation to update
		 * @param	delta the elapsed time in seconds since the last update */
		static void updateAnimation(Animation& animation, float delta);

		/** Returns the previous and next KeyFrames to the given timePoint
		 *
		 * @param	keyFrames the KeyFrames to check for. They must be sorted
		 *			ascendently by their timePoints
		 * @param	timePoint the time in seconds with which we want to search
		 *			the KeyFrames
		 * @return	a pair with the previous and next KeyFrames */
		static std::pair<KeyFrame, KeyFrame> getPreviousAndNextKeyFrames(
			const std::vector<KeyFrame>& keyFrames, float timePoint
		);
	};

}

#endif		// ANIMATION_SYSTEM_H
