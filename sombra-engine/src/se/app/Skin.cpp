#include <glm/gtc/matrix_transform.hpp>
#include "se/app/Skin.h"

namespace se::app {

	std::vector<glm::mat4> calculateJointMatrices(const Skin& skin, const glm::mat4& modelMatrix)
	{
		std::vector<glm::mat4> jointMatrices(skin.inverseBindMatrices.size());

		glm::mat4 invertedModelMatrix = glm::inverse(modelMatrix);

		for (const auto& [node, nodeIndex] : skin.jointIndices) {
			// Get the joint global transforms
			glm::mat4 translation	= glm::translate(glm::mat4(1.0f), node->getData().worldTransforms.position);
			glm::mat4 rotation		= glm::mat4_cast(node->getData().worldTransforms.orientation);
			glm::mat4 scale			= glm::scale(glm::mat4(1.0f), node->getData().worldTransforms.scale);
			glm::mat4 nodeWorldTransforms = translation * rotation * scale;

			jointMatrices[nodeIndex] = invertedModelMatrix * nodeWorldTransforms * skin.inverseBindMatrices[nodeIndex];
		}

		return jointMatrices;
	}

}
