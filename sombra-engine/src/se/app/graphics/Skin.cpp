#include <glm/gtc/matrix_transform.hpp>
#include "se/app/graphics/Skin.h"

namespace se::app {

	utils::FixedVector<glm::mat4, Skin::kMaxJoints> calculateJointMatrices(const Skin& skin, const glm::mat4& modelMatrix)
	{
		utils::FixedVector<glm::mat4, Skin::kMaxJoints> jointMatrices(skin.inverseBindMatrices.size());

		glm::mat4 invertedModelMatrix = glm::inverse(modelMatrix);
		for (const auto& [node, nodeIndex] : skin.jointIndices) {
			jointMatrices[nodeIndex] = invertedModelMatrix * node->getData().worldMatrix * skin.inverseBindMatrices[nodeIndex];
		}

		return jointMatrices;
	}

}
