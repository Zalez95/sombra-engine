#ifndef SKIN_H
#define SKIN_H

#include <map>
#include "../animation/AnimationNode.h"

namespace se::app {

	/**
	 * Struct Skin, holds the relation between the AnimationNodes and the
	 * joints of the skeletal animation
	 */
	struct Skin
	{
		/** A pointer to the AnimationNode used as the root joint of the
		 * skeleton */
		animation::AnimationNode* skeletonRoot;

		/** Maps the AnimationNodes with their respective joint indices */
		std::map<animation::AnimationNode*, int> jointIndices;

		/** The inverse bind matrices of the joints */
		std::vector<glm::mat4> inverseBindMatrices;
	};

}

#endif		// SKIN_H
