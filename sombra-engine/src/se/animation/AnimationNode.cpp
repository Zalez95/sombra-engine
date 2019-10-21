#include "se/animation/AnimationNode.h"

namespace se::animation {

	void updateWorldTransforms(AnimationNode& rootNode)
	{
		for (auto itNode = rootNode.begin(); itNode != rootNode.end(); ++itNode) {
			NodeData& currentData = itNode->getData();
			AnimationNode* parent = itNode->getParent();
			if (parent) {
				// Update the current world transforms with the parent
				// world transforms
				NodeData& parentData = parent->getData();
				currentData.worldTransforms.position = parentData.worldTransforms.position + currentData.localTransforms.position;
				currentData.worldTransforms.orientation = parentData.worldTransforms.orientation * currentData.localTransforms.orientation;
				currentData.worldTransforms.scale = parentData.worldTransforms.scale * currentData.localTransforms.scale;
			}
			else {
				currentData.worldTransforms = currentData.localTransforms;
			}

			currentData.worldTransformsUpdated = true;
		}
	}

}
