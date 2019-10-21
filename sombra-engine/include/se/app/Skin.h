#ifndef SKIN_H
#define SKIN_H

#include <map>
#include <vector>
#include "../animation/AnimationNode.h"

namespace se::app {

	/**
	 * Struct Skin, holds the relationship between the AnimationNodes and the
	 * joints of a Renderable3D used for the skinning of its Mesh
	 */
	struct Skin
	{
		/** Maps the AnimationNodes with their respective joint indices */
		std::map<animation::AnimationNode*, int> jointIndices;

		/** The inverse bind matrices of the joints, they're used to transform
		 * the mesh to the local space of each joint */
		std::vector<glm::mat4> inverseBindMatrices;
	};


	/** Calculates the joint matrices of the given skin
	 *
	 * @param	skin the Skin to calculate its joint matrices
	 * @param	modelMatrix the model matrix that transforms from the local
	 *			space of a Renderable3D to the global space
	 * @return	a vector with the joint matrices of the Skin */
	std::vector<glm::mat4> calculateJointMatrices(
		const Skin& skin, const glm::mat4& modelMatrix
	);

}

#endif		// SKIN_H
