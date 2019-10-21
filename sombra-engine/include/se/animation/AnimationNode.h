#ifndef ANIMATION_NODE_H
#define ANIMATION_NODE_H

#include <string>
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
	};


	/**
	 * Struct NodeData. It holds the data of an AnimationNode.
	 */
	struct NodeData
	{
		/** The name of the Node */
		std::string name;

		/** The node transforms in relation to its parent */
		NodeTransforms localTransforms;

		/** The node transforms in world space */
		NodeTransforms worldTransforms;

		/** If the node has been updated by the AnimationSystem or not */
		bool animated;

		/** If the world transform of the node has been updated by the
		 * AnimationSystem or not */
		bool worldTransformsUpdated;

		/** Creates a new NodeData
		 *
		 * @param	name the name of the NodeData, empty by default */
		NodeData(const std::string& name = "") :
			name(name), animated(false), worldTransformsUpdated(false) {};
	};


	using AnimationNode = utils::TreeNode<NodeData>;


	/** Updates the world transforms of the given AnimationNode and its
	 * descendants with the changes made to their parents or local transforms
	 *
	 * @param	node the root AnimationNode of the hierarchy of Nodes to
	 *			update */
	void updateWorldTransforms(AnimationNode& rootNode);

}

#endif		// ANIMATION_NODE_H
