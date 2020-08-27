#ifndef I_ANIMATOR_H
#define I_ANIMATOR_H

namespace se::animation {

	/**
	 * Class IAnimator, it's used to apply animations to AnimationNodes
	 */
	class IAnimator
	{
	public:		// Functions
		/** Class destructor */
		virtual ~IAnimator() = default;

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

		/** Resets the animate state of every node */
		virtual void resetNodesAnimatedState() = 0;

		/** Updates the IAnimator's nodes world transforms (and their
		 * descendants) with the changes made by the animations */
		virtual void updateNodesWorldTransforms() = 0;
	};

}

#endif		// I_ANIMATOR_H
