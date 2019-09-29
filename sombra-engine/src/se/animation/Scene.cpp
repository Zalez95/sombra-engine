#include <glm/gtc/matrix_transform.hpp>
#include "se/animation/Scene.h"

namespace se::animation {

	SceneNode::SceneNode(const NodeData& data) : utils::TreeNode<NodeData>(data) {}


	glm::mat4 SceneNode::getLocalMatrix() const
	{
		glm::mat4 translation	= glm::translate(glm::mat4(1.0f), getData().position);
		glm::mat4 rotation		= glm::mat4_cast(getData().orientation);
		glm::mat4 scale			= glm::scale(glm::mat4(1.0f), getData().scale);
		return translation * rotation * scale;
	}

}
