#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "../utils/TreeNode.h"

namespace se::animation {

	/**
	 * Struct NodeData. It holds the data of an SceneNode.
	 */
	struct NodeData
	{
		/** The name of the Node */
		std::string name;

		/** The Node position */
		glm::vec3 position;

		/** The Node orientation */
		glm::quat orientation;

		/** The Node scale */
		glm::vec3 scale;
	};


	/**
	 * Class SceneNode. It's one of the nodes in the Scene Graph. The NodeData
	 * of each SceneNode its stored in relation to its parent SceneNode.
	 */
	class SceneNode : public utils::TreeNode<NodeData>
	{
	public:		// Functions
		/** Creates a new SceneNode
		 *
		 * @param	data the NodeData of the SceneNode */
		SceneNode(const NodeData& data);

		/** @return	the transforms matrix in local space of the current
		 *			SceneNode calculated from its NodeData */
		glm::mat4 getLocalMatrix() const;
	};


	/**
	 * Class Scene, holds the Scene Graph
	 */
	struct Scene
	{
		/** The name of the Scene */
		std::string name;

		/** The root nodes of the Scene */
		std::vector<SceneNode> mRootNodes;
	};

}

#endif		// SCENE_H
