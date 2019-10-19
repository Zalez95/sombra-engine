#include "se/animation/Animators.h"

namespace se::animation {

	void Vec3Animator::animate(float elapsedTime)
	{
		mAccumulatedTime += elapsedTime;
		glm::vec3 transformation = mAnimation->interpolate(mAccumulatedTime);

		for (auto& animationNode : mNodes) {
			switch (animationNode.type) {
				case TransformationType::Translation:
					animationNode.node->getData().position += transformation;
					break;
				case TransformationType::Scale:
					animationNode.node->getData().scale += transformation;
					break;
				default:
					break;
			}
		}
	}


	void QuatAnimator::animate(float elapsedTime)
	{
		mAccumulatedTime += elapsedTime;
		glm::quat transformation = mAnimation->interpolate(mAccumulatedTime);

		for (auto& animationNode : mNodes) {
			switch (animationNode.type) {
				case TransformationType::Rotation:
					animationNode.node->getData().orientation *= transformation;
					break;
				default:
					break;
			}
		}
	}

}
