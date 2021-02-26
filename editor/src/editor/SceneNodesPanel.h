#ifndef SCENE_NODES_PANEL_H
#define SCENE_NODES_PANEL_H

#include <se/animation/AnimationNode.h>

namespace editor {

	class Editor;


	/**
	 * Class SceneNodesPanel, it's the ImGui panel used for viewing and
	 * interacting with the Scene AnimationNodes
	 */
	class SceneNodesPanel
	{
	private:	// Nested types
		using NodeIterator =
			se::animation::AnimationNode::iterator<se::utils::Traversal::BFS>;

	private:	// Attributes
		/** A reference to the Editor that holds the SceneNodesPanel */
		Editor& mEditor;

		/** The selected node */
		NodeIterator mSelectedNode;

		/** The type of orientation to show */
		int mOrientationType;

		/** The NodeData where the user input will be stored */
		se::animation::NodeData mWorkingData;

		/** The operations to execute */
		bool mRemove, mAdd, mRename, mRemoveHierarchy, mChangeParent;

		/** If the operation should affect the descendant nodes or not */
		bool mDescendants;

		/** If we should use the root node in the operation or not */
		bool mRoot;

	public:		// Functions
		/** Creates a new SceneNodesPanel
		 *
		 * @param	editor a reference to the Editor that holds the Entities */
		SceneNodesPanel(Editor& editor) :
			mEditor(editor), mSelectedNode(nullptr), mOrientationType(0),
			mRemove(false), mAdd(false), mRename(false),
			mRemoveHierarchy(false), mChangeParent(false),
			mDescendants(false), mRoot(false) {};

		/** Draws the current panel */
		void render();
	private:
		/** Draws the given node
		 *
		 * @param	itNode an iterator to the node to draw */
		void drawNode(NodeIterator itNode);

		/** Checks if the current @see mWorkingData name isn't already used.
		 * If it's already used it will add a number to it */
		void fixWorkingDataName();
	};

}

#endif		// SCENE_NODES_PANEL_H
