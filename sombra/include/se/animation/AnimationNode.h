#ifndef ANIMATION_NODE_H
#define ANIMATION_NODE_H

#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "../utils/TreeNode.h"

namespace se::animation {

	/**
	 * Struct NodeTransforms, holds all the transforms of a Node
	 */
	struct NodeTransforms
	{
		/** The position transformation */
		glm::vec3 position;

		/** The orientation transformation */
		glm::quat orientation;

		/** The scale transformation */
		glm::vec3 scale;

		/** Creates a new NodeTransforms */
		NodeTransforms() :
			position(0.0f), orientation(1.0f, glm::vec3(0.0f)), scale(1.0f) {};
		NodeTransforms(
			const glm::vec3& position,
			const glm::quat& orientation,
			const glm::vec3& scale
		) : position(position), orientation(orientation), scale(scale) {};
	};


	/**
	 * Struct NodeData. It holds the data of an AnimationNode.
	 */
	struct NodeData
	{
		/** The maximum name length allowed */
		static constexpr std::size_t kMaxLength = 256;

		/** The name of the Node */
		std::array<char, kMaxLength> name = {};

		/** The node transforms in relation to its parent */
		NodeTransforms localTransforms;

		/** The node transforms in world space */
		NodeTransforms worldTransforms;

		/** The node transforms matrix in world space */
		glm::mat4 worldMatrix = glm::mat4(1.0f);

		/** If the node has been updated by the AnimationSystem or not */
		bool animated = false;

		/** If the world transform of the node has been updated by the
		 * AnimationSystem or not */
		bool worldTransformsUpdated = false;
	};


	using AnimationNode = utils::TreeNode<NodeData>;


	/** Updates the world transforms of the given AnimationNode and its
	 * descendants with the changes made to their parents or local transforms
	 *
	 * @param	rootNode the root AnimationNode of the hierarchy of Nodes to
	 *			update */
	void updateWorldTransforms(AnimationNode& rootNode);


	/** Updates the animated state of the given AnimationNode and its
	 * descendants
	 *
	 * @param	rootNode the root AnimationNode of the hierarchy of Nodes to
	 *			update
	 * @param	animated the new animated state of the nodes */
	void updateAnimatedState(AnimationNode& rootNode, bool animated);

}

#endif		// ANIMATION_NODE_H
