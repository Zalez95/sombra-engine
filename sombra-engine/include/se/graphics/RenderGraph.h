#ifndef RENDER_GRAPH_H
#define RENDER_GRAPH_H

#include "RenderNode.h"

namespace se::graphics {

	/**
	 * Class RenderGraph, it's a directed graph in which RenderNodes represents
	 * each of the nodes of the graph and their Input and Output connectors act
	 * as the edges. The RenderGraph is needed for ensuring the order of
	 * execution of the different RenderNodes. The RenderGraph is used for
	 * implementing multi-pass Techiques.
	 * Every RenderGraph by default has a "resources" RenderNode where all the
	 * global Bindable resources can accessed from other RenderNodes. This
	 * "resources" RenderNode also has a "defaultFB" output with the default
	 * FrameBuffer to draw to.
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

		/** Searchs a RenderNode with the same name in the RenderGraph
		 *
		 * @param	nodeName the name of the RenderNode
		 * @return	a pointer to the RenderNode, nullptr if it wasn't found */
		RenderNode* getNode(const std::string& nodeName) const;

		/** Prepares the Graph for executing it after adding all its
		 * RenderNodes. This function must be called once before executing the
		 * Graph */
		void prepareGraph();

		/** Executes the RenderNodes added to the RenderGraph */
		void execute();
	};

}

#endif		// RENDER_GRAPH_H
