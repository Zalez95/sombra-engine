#ifndef SCENE_NODES_PANEL_H
#define SCENE_NODES_PANEL_H

#include <se/animation/AnimationNode.h>
#include "IEditorPanel.h"

namespace editor {

	/**
	 * Class SceneNodesPanel, it's the ImGui panel used for viewing and
	 * interacting with the Scene AnimationNodes
	 */
	class SceneNodesPanel : public IEditorPanel
	{
	private:	// Nested types
		using NodeIterator =
			se::animation::AnimationNode::iterator<se::utils::Traversal::BFS>;

	private:	// Attributes
		/** The selected node */
		NodeIterator mSelectedNode;

		/** The type of orientation to show */
		int mOrientationType = 0;

		/** The NodeData where the user input will be stored */
		se::animation::NodeData mWorkingData;

		/** The operations to execute */
		bool mRemove = false, mAdd = false, mRemoveHierarchy = false,
			mChangeParent = false;

		/** If the operation should affect the descendant nodes or not */
		bool mDescendants = false;

		/** If we should use the root node in the operation or not */
		bool mRoot = false;

	public:		// Functions
		/** Creates a new SceneNodesPanel
		 *
		 * @param	editor a reference to the Editor that holds the Panel */
		SceneNodesPanel(Editor& editor) : IEditorPanel(editor) {};

		/** @copydoc IEditorPanel::render() */
		virtual bool render() override;
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
