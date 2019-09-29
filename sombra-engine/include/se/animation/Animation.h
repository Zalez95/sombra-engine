#ifndef ANIMATION_H
#define ANIMATION_H

#include <map>
#include <vector>
#include "Scene.h"

namespace se::animation {

	/**
	 * Struct KeyFrame, its used to represent the change of state of an Object
	 * at some specified time.
	 */
	struct KeyFrame
	{
		/** The 3D translation of the Object in relation to its initial
		 * location */
		glm::vec3 translation;

		/** The 3D rotation of the Object in relation to its initial
		 * orientation */
		glm::quat rotation;

		/** The 3D scale of the Object in relation to its initial size */
		glm::vec3 scale;

		/** The time point in seconds since the start of the Animation of the
		 * KeyFrame. */
		float timePoint;

		/** Creates a new KeyFrame with no transformation at the second 0 */
		KeyFrame();

		/** Creates a new KeyFrame
		 *
		 * @param	translation the translation of the KeyFrame
		 * @param	rotation the rotation of the KeyFrame
		 * @param	scale the scale of the KeyFrame
		 * @param	timePoint the time point of the KeyFrame */
		KeyFrame(
			const glm::vec3& translation,
			const glm::quat& rotation,
			const glm::vec3& scale,
			float timePoint
		);
	};


	/**
	 * Struct Animation, it holds the set of KeyFrames used to animate a
	 * herarchy of Nodes
	 */
	class Animation
	{
	private:	// Attributes
		friend class AnimationSystem;
		using KeyFrames = std::vector<KeyFrame>;

		/** Maps the SceneNodes with their respective KeyFrames in the
		 * Animation. The KeyFrames are sorted ascendently by their timePoint */
		std::map<SceneNode*, KeyFrames> mNodeKeyFrames;

		/** The length of the Animation in seconds */
		float mLength;

		/** If the animation has to restart when the animation length is
		 * reached */
		bool mLoopAnimation;

		/** The elapsed time in seconds since the start of the Animation */
		float mAccumulatedTime;

	public:		// Functions
		/** Creates a new Animation
		 *
		 * @param	length the length of the Animation in seconds
		 * @param	loop if the animation has to restart or not */
		Animation(float length, bool loop);

		/** Adds a new KeyFrame to the Animation
		 *
		 * @param	node a pointer to the SceneNode related to the given
		 *			KeyFrame
		 * @param	keyFrame the KeyFrame to add to the Animation */
		void addKeyFrame(SceneNode* node, const KeyFrame& keyFrame);
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
