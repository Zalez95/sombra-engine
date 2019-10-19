#include <glm/gtc/matrix_transform.hpp>
#include "se/animation/AnimationNode.h"

namespace se::animation {

	NodeData::NodeData() : name(), position(0.0f), orientation(1.0f, glm::vec3(0.0f)), scale(1.0f), changed(true)
	{
		worldTransforms = getLocalMatrix(*this);
	}


	glm::mat4 getLocalMatrix(const NodeData& data)
	{
		glm::mat4 translation	= glm::translate(glm::mat4(1.0f), data.position);
		glm::mat4 rotation		= glm::mat4_cast(data.orientation);
		glm::mat4 scale			= glm::scale(glm::mat4(1.0f), data.scale);
		return translation * rotation * scale;
	}


	void updateWorldMatrices(AnimationNode& rootNode)
	{
		for (auto itNode = rootNode.begin(); itNode != rootNode.end(); ++itNode) {
			NodeData& currentData = itNode->getData();
			AnimationNode* parent = itNode->getParent();
			if (parent) {
				NodeData& parentData = parent->getData();
				if (currentData.changed || parentData.changed) {
					// Update the current world transforms with the parent
					// world transforms
					glm::mat4 parentTransforms = parentData.worldTransforms;
					currentData.worldTransforms = parentTransforms * getLocalMatrix(currentData);
					currentData.changed = true;
				}
			}
			else if (currentData.changed) {
				currentData.worldTransforms = getLocalMatrix(currentData);
			}
		}
	}

}
