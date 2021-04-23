#include <cstring>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "se/app/SkinComponent.h"

namespace se::app {

	SkinComponent SkinComponent::duplicateSkinComponent(animation::AnimationNode* otherRootNode) const
	{
		MapNodeJoint otherJointIndices;
		for (auto [myNode, index] : mJointIndices) {
			auto it = std::find_if(otherRootNode->begin(), otherRootNode->end(),
				[&, myNode = myNode](const animation::AnimationNode& otherNode) {
					return std::strcmp(myNode->getData().name.data(), otherNode.getData().name.data());
				}
			);
			if (it != otherRootNode->end()) {
				otherJointIndices.emplace_back(&(*it), index);
			}
		}

		return SkinComponent(otherRootNode, mSkin, otherJointIndices);
	}


	utils::FixedVector<glm::mat3x4, Skin::kMaxJoints> SkinComponent::calculateJointMatrices(const glm::mat4& modelMatrix)
	{
		utils::FixedVector<glm::mat3x4, Skin::kMaxJoints> jointMatrices(mSkin->inverseBindMatrices.size());

		glm::mat4 invertedModelMatrix = glm::inverse(modelMatrix);
		for (const auto& [node, nodeIndex] : mJointIndices) {
			glm::mat4 localNodeMatrix = invertedModelMatrix * node->getData().worldMatrix;
			jointMatrices[nodeIndex] = glm::transpose(localNodeMatrix * mSkin->inverseBindMatrices[nodeIndex]);
		}

		return jointMatrices;
	}

}
