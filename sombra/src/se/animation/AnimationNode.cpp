#include "se/animation/AnimationNode.h"
#include "se/utils/MathUtils.h"
#include <glm/gtc/matrix_transform.hpp>

namespace se::animation {

	void updateWorldTransformsSingle(AnimationNode& node);


	void updateWorldTransforms(AnimationNode& rootNode)
	{
		updateWorldTransformsSingle(rootNode);
		for (auto itNode = rootNode.begin(); itNode != rootNode.end(); ++itNode) {
			updateWorldTransformsSingle(*itNode);
		}
	}


	void updateAnimatedState(AnimationNode& rootNode, bool animated)
	{
		rootNode.getData().animated = animated;
		for (auto itNode = rootNode.begin(); itNode != rootNode.end(); ++itNode) {
			itNode->getData().animated = animated;
		}
	}


	void updateWorldTransformsSingle(AnimationNode& node)
	{
		NodeData& currentData = node.getData();
		AnimationNode* parent = node.getParent();

		glm::mat4 translation	= glm::translate(glm::mat4(1.0f), currentData.localTransforms.position);
		glm::mat4 rotation		= glm::mat4_cast(currentData.localTransforms.orientation);
		glm::mat4 scale			= glm::scale(glm::mat4(1.0f), currentData.localTransforms.scale);
		glm::mat4 localMatrix	= translation * rotation * scale;

		if (parent) {
			// Update the current world transforms with the parent
			// world transforms
			NodeData& parentData = parent->getData();
			currentData.worldMatrix = parentData.worldMatrix * localMatrix;
			currentData.worldTransforms.position = parentData.worldTransforms.position + currentData.localTransforms.position;
			currentData.worldTransforms.orientation = parentData.worldTransforms.orientation * currentData.localTransforms.orientation;
			currentData.worldTransforms.scale = parentData.worldTransforms.scale * currentData.localTransforms.scale;
		}
		else {
			currentData.worldMatrix = localMatrix;
			currentData.worldTransforms = currentData.localTransforms;
		}

		currentData.worldTransformsUpdated = true;
	}

}
