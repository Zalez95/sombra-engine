#ifndef ANIMATION_NODE_H
#define ANIMATION_NODE_H

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "../utils/TreeNode.h"

namespace se::animation {

	/**
	 * Struct NodeData. It holds the data of an AnimationNode.
	 */
	struct NodeData
	{
		/** The name of the Node */
		std::string name;

		/** The Node position in relation to the parent Node */
		glm::vec3 position;

		/** The Node orientation in relation to the parent Node */
		glm::quat orientation;

		/** The Node scale in relation to the parent Node */
		glm::vec3 scale;

		/** If the Node data has been updated or not */
		bool changed;

		/** The transformations of the Node in world space */
		glm::mat4 worldTransforms;

		/** Creates a new NodeData */
		NodeData();
	};


	using AnimationNode = utils::TreeNode<NodeData>;


	/** Calculates the local transformation matrix of the given NodeData
	 *
	 * @param	data the node data with the local transformations
	 * @return	the local transformations matrix */
	glm::mat4 getLocalMatrix(const NodeData& data);


	/** Updates the world matrices of the given AnimationNode and its
	 * descendants with the changes made to their parents or local transforms
	 *
	 * @param	node the root AnimationNode of the hierarchy of Nodes to
	 *			update */
	void updateWorldMatrices(AnimationNode& rootNode);

}

#endif		// ANIMATION_NODE_H
