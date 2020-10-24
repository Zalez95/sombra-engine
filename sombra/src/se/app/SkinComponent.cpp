#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "se/app/SkinComponent.h"

namespace se::app {

	SkinComponent SkinComponent::duplicateSkinComponent(
		animation::AnimationNode* sourceRootNode,
		animation::AnimationNode* copyRootNode
	) {
		SkinComponent ret(mSkin);
		ret.mJointIndices.resize(mJointIndices.size());

		auto itS = sourceRootNode->begin(), itC = copyRootNode->begin();
		while (itS != sourceRootNode->end()) {
			auto itJointIndex = std::find_if(mJointIndices.begin(), mJointIndices.end(), [&](const auto& pair) {
				return pair.first == &(*itS);
			});
			if (itJointIndex != mJointIndices.end()) {
				auto pos = std::distance(mJointIndices.begin(), itJointIndex);
				ret.mJointIndices[pos] = { &(*itC), itJointIndex->second };
			}

			++itS; ++itC;
		}

		return ret;
	}


	utils::FixedVector<glm::mat4, Skin::kMaxJoints> SkinComponent::calculateJointMatrices(const glm::mat4& modelMatrix)
	{
		utils::FixedVector<glm::mat4, Skin::kMaxJoints> jointMatrices(mSkin->inverseBindMatrices.size());

		glm::mat4 invertedModelMatrix = glm::inverse(modelMatrix);
		for (const auto& [node, nodeIndex] : mJointIndices) {
			glm::mat4 localNodeMatrix = invertedModelMatrix * node->getData().worldMatrix;
			jointMatrices[nodeIndex] = localNodeMatrix * mSkin->inverseBindMatrices[nodeIndex];
		}

		return jointMatrices;
	}

}
