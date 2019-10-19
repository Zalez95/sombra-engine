#ifndef I_ANIMATORS_H
#define I_ANIMATORS_H

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "IAnimator.h"
#include "IAnimation.h"

namespace se::animation {

	/**
	 * Class Vec3Animator, it's an IAnimator used to apply vec3 transformations
	 * to its AnimationNodes
	 */
	class Vec3Animator : public IAnimator
	{
	private:	// Nested types
		using Vec3AnimationSPtr = std::shared_ptr<IAnimation<glm::vec3>>;

	private:	// Attributes
		/** The animation of the Animator */
		Vec3AnimationSPtr mAnimation;

	public:		// Functions
		/** Creates a new Vec3Animator
		 *
		 * @param	animation a pointer to the animation of the Vec3Animator */
		Vec3Animator(Vec3AnimationSPtr animation)
			: IAnimator(), mAnimation(animation) {};

		/** Applies the animation to the nodes of the Animator
		 *
		 * @param	elapsedTime the time elapsed since the last call to the
		 *			function
		 * @note	the animation will be applied only to the AnimationNodes
		 *			added with a Translation or Scale transformation */
		virtual void animate(float elapsedTime) override;
	};


	/**
	 * Class QuatAnimator, it's an IAnimator used to apply quat transformations
	 * to its AnimationNodes
	 */
	class QuatAnimator : public IAnimator
	{
	private:	// Nested types
		using QuatAnimationSPtr = std::shared_ptr<IAnimation<glm::quat>>;

	private:	// Attributes
		/** The animation of the Animator */
		QuatAnimationSPtr mAnimation;

	public:		// Functions
		/** Creates a new QuatAnimator
		 *
		 * @param	animation a pointer to the animation of the QuatAnimator */
		QuatAnimator(QuatAnimationSPtr animation)
			: IAnimator(), mAnimation(animation) {};

		/** Applies the animation to the nodes of the Animator
		 *
		 * @param	elapsedTime the time elapsed since the last call to the
		 *			function
		 * @note	the animation will be applied only to the AnimationNodes
		 *			added with a Rotation transformation */
		virtual void animate(float elapsedTime) override;
	};

}

#endif		// ANIMATORS_H
