#include "se/animation/Animators.h"

namespace se::animation {

	void Vec3Animator::animate(float elapsedTime)
	{
		mAccumulatedTime += elapsedTime;
		glm::vec3 transformation = mAnimation->interpolate(mAccumulatedTime);

		for (auto& animationNode : mNodes) {
			switch (animationNode.type) {
				case TransformationType::Translation:
					animationNode.node->getData().localTransforms.position += transformation;
					animationNode.node->getData().animated = true;
					animationNode.node->getData().worldTransformsUpdated = false;
					break;
				case TransformationType::Scale:
					animationNode.node->getData().localTransforms.scale *= transformation;
					animationNode.node->getData().animated = true;
					animationNode.node->getData().worldTransformsUpdated = false;
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
					animationNode.node->getData().localTransforms.orientation *= transformation;
					animationNode.node->getData().animated = true;
					animationNode.node->getData().worldTransformsUpdated = false;
					break;
				default:
					break;
			}
		}
	}

}
