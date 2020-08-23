#ifndef SKIN_H
#define SKIN_H

#include <unordered_map>
#include "../../utils/FixedVector.h"
#include "../../animation/AnimationNode.h"

namespace se::app {

	/**
	 * Struct Skin, holds the relationship between the AnimationNodes and the
	 * joints of a Renderable3D used for the skinning of its Mesh
	 */
	struct Skin
	{
		/** The maximum number of joints in the skin */
		static constexpr unsigned int kMaxJoints = 64;

		/** Maps the AnimationNodes with their respective joint indices */
		std::unordered_map<animation::AnimationNode*, std::size_t> jointIndices;

		/** The inverse bind matrices of the joints, they're used for
		 * transforming the mesh to the local space of each joint */
		utils::FixedVector<glm::mat4, kMaxJoints> inverseBindMatrices;
	};


	/** Calculates the joint matrices of the given skin
	 *
	 * @param	skin the Skin to calculate its joint matrices
	 * @param	modelMatrix the model matrix that transforms from the local
	 *			space of a Renderable3D to the global space
	 * @return	a vector with the joint matrices of the Skin */
	utils::FixedVector<glm::mat4, Skin::kMaxJoints> calculateJointMatrices(
		const Skin& skin, const glm::mat4& modelMatrix
	);

}

#endif		// SKIN_H
