#ifndef ANIMATION_H
#define ANIMATION_H

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace se::animation {

	/**
	 * Struct Bone, it represents each part of the rig of mesh
	 */
	struct Bone
	{
		/** The name used to identify the Bone */
		std::string name;

		/** The 3D scale of the Bone */
		glm::vec3 scale;

		/** The 3D orientation of the Bone */
		glm::quat orientation;

		/** The 3D position of the Bone */
		glm::vec3 position;
	};


	/**
	 * Struct KeyFrame, its used to represent the change of state of an Object
	 * at some specified time.
	 */
	struct KeyFrame
	{
		/** The 3D scale of the Object in relation to its initial size */
		glm::vec3 scale;

		/** The 3D rotation of the Object in relation to its initial
		 * orientation */
		glm::quat rotation;

		/** The 3D translation of the Object in relation to its initial
		 * location */
		glm::vec3 translation;

		/** The time point in seconds since the start of the Animation of the
		 * KeyFrame. */
		float timePoint;

		/** Creates a new KeyFrame with no transformation at the second 0 */
		KeyFrame();

		/** Creates a new KeyFrame
		 *
		 * @param	scale the scale of the KeyFrame
		 * @param	rotation the rotation of the KeyFrame
		 * @param	translation the translation of the KeyFrame
		 * @param	timePoint the time point of the KeyFrame */
		KeyFrame(
			const glm::vec3& scale,
			const glm::quat& rotation,
			const glm::vec3& translation,
			float timePoint
		);
	};


	/**
	 * Struct Animation, it holds the set of KeyFrames that composes the
	 * animation of a Bone
	 */
	class Animation
	{
	private:	// Attributes
		friend class AnimationSystem;

		/** The initial Bone state of the Animation */
		Bone mInitialBone;

		/** The length of the Animation in seconds */
		float mLength;

		/** If the animation has to restart when the animation length is
		 * reached */
		bool mLoopAnimation;

		/** The KeyFrames of the Animation, sorted ascendently by their
		 * timePoint */
		std::vector<KeyFrame> mKeyFrames;

		/** The elapsed time in seconds since the start of the Animation */
		float mAccumulatedTime;

		/** The current Bone state of the Animation */
		Bone mCurrentBone;

	public:		// Functions
		/** Creates a new Animation
		 *
		 * @param	bone the Bone to animate
		 * @param	length the length of the Animation in seconds
		 * @param	loop if the animation has to restart or not */
		Animation(const Bone& bone, float length, bool loop);

		/** @return	the length in seconds of the Animation */
		float getLength() const { return mLength; };

		/** @return	the current Skeleton pose of the animation */
		const Bone& getCurrentBone() const { return mCurrentBone; };

		/** Adds a new KeyFrame to the Animation
		 *
		 * @param	keyFrame the KeyFrame to add to the Animation */
		void addKeyFrame(const KeyFrame& keyFrame);
	};


	/** Creates a new KeyFrame by applying linear interpolation to the data of
	 * the first and second KeyFrames by the given factor
	 *
	 * @param	k1 the first KeyFrame data
	 * @param	k2 the second KeyFrame data
	 * @param	factor indicates how much influence will have the k1 or k2 data
	 *			in the new KeyFrame. Its value must be between 0 and 1, the
	 *			closer to 0 the closer will be the new KeyFrame to the first one
	 * @return	the interpolated KeyFrame */
	KeyFrame keyFrameLinearInterpolation(
		const KeyFrame& k1, const KeyFrame& k2, float factor
	);

}

#endif		// ANIMATION_H
