#include "se/animation/AnimationNode.h"
#include "se/utils/MathUtils.h"
#include <glm/gtc/matrix_transform.hpp>

namespace se::animation {

	void updateWorldTransforms(AnimationNode& rootNode)
	{
		for (auto itNode = rootNode.begin(); itNode != rootNode.end(); ++itNode) {
			NodeData& currentData = itNode->getData();
			AnimationNode* parent = itNode->getParent();

			glm::mat4 translation	= glm::translate(glm::mat4(1.0f), currentData.localTransforms.position);
			glm::mat4 rotation		= glm::mat4_cast(currentData.localTransforms.orientation);
			glm::mat4 scale			= glm::scale(glm::mat4(1.0f), currentData.localTransforms.scale);
			glm::mat4 localMatrix	= translation * rotation * scale;

			if (parent) {
				// Update the current world transforms with the parent
				// world transforms
				currentData.worldMatrix = parent->getData().worldMatrix * localMatrix;
				utils::decompose(
					currentData.worldMatrix,
					currentData.worldTransforms.position,
					currentData.worldTransforms.orientation,
					currentData.worldTransforms.scale
				);
			}
			else {
				currentData.worldMatrix = localMatrix;
				currentData.worldTransforms = currentData.localTransforms;
			}

			currentData.worldTransformsUpdated = true;
		}
	}

}
