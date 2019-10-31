#include <glm/gtc/matrix_transform.hpp>
#include "se/app/Skin.h"

namespace se::app {

	std::vector<glm::mat4> calculateJointMatrices(const Skin& skin, const glm::mat4& modelMatrix)
	{
		std::vector<glm::mat4> jointMatrices(skin.inverseBindMatrices.size());

		glm::mat4 invertedModelMatrix = glm::inverse(modelMatrix);

		for (const auto& [node, nodeIndex] : skin.jointIndices) {
			jointMatrices[nodeIndex] = invertedModelMatrix * node->getData().worldMatrix * skin.inverseBindMatrices[nodeIndex];
		}

		return jointMatrices;
	}

}
