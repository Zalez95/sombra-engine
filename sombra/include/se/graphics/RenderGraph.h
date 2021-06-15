#ifndef RENDER_GRAPH_H
#define RENDER_GRAPH_H

#include "RenderNode.h"

namespace se::graphics {

	/**
	 * Class RenderGraph, it's a directed graph in which RenderNodes represents
	 * each of the nodes of the graph and their Input and Output connectors act
	 * as the edges. The RenderGraph is needed for implementing multi-pass
	 * Techiques because it ensures the order of execution of the different
	 * RenderNodes.
	 * Every RenderGraph by default has a "resources" BindableRenderNode where
	 * all the global Bindable resources can be accessed from other RenderNodes.
	 * This "resources" RenderNode also has a "defaultFB" output with the
	 * default FrameBuffer to draw to.
	 * The RenderGraph nodes will be executed from top to bottom, this means
	 * that the parent nodes will be executed prior to the child ones. If some
	 * nodes are at the same level, they will be executed in the same order
	 * that they've added to the RenderGraph.
	 */
	class RenderGraph
	{
	private:	// Nested types
		using RenderNodeUPtr = std::unique_ptr<RenderNode>;
		class ResourceNode;
		struct SortStackContent;

	private:	// Attributes
		/** All the RenderNodes of the RenderGraph */
		std::vector<RenderNodeUPtr> mRenderNodes;

	public:		// Functions
		/** Creates a new RenderGraph */
		RenderGraph();

		/** Class destructor */
		virtual ~RenderGraph() = default;

		/** Adds the given RenderNode to the RenderGraph
		 *
		 * @param	node a pointer to the new RenderNode in the RenderGraph
		 * @return	true if the RenderNode was added successfully, false
		 *			otherwise */
		bool addNode(RenderNodeUPtr node);

		/** Iterates through all the RenderNodes of the RenderGraph calling
		 * the given callback function
		 *
		 * @param	callback the function to call for each RenderNode */
		template <typename F>
		void processNodes(F callback) const;

		/** Removes the given RenderNode from the RenderGraph
		 *
		 * @param	node a pointer to the RenderNode to remove
		 * @return	true if the RenderNode was removed successfully, false
		 *			otherwise */
		bool removeNode(RenderNode* node);

		/** Removes all the RenderNodes from the RenderGraph */
		void clearNodes();

		/** Searchs a RenderNode with the same name in the RenderGraph
		 *
		 * @param	nodeName the name of the RenderNode
		 * @return	a pointer to the RenderNode, nullptr if it wasn't found */
		RenderNode* getNode(const std::string& nodeName) const;

		/** Prepares the Graph for executing it after adding all its
		 * RenderNodes
		 *
		 * @note this function must be called before calling @see execute
		 * each time a node is added or removed from the RenderGraph */
		void prepareGraph();

		/** Executes the RenderNodes added to the RenderGraph */
		void execute();
	};


	template <typename F>
	void RenderGraph::processNodes(F callback) const
	{
		for (auto& node : mRenderNodes) {
			callback(node.get());
		}
	}

}

#endif		// RENDER_GRAPH_H
