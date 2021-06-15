#include <algorithm>
#include "se/graphics/RenderGraph.h"
#include "se/graphics/BindableRenderNode.h"
#include "se/graphics/core/FrameBuffer.h"
#include "se/utils/Log.h"

namespace se::graphics {

	/** Class ResourceNode, it's a BindableRenderNode that just holds
	 * Bindable resources */
	class RenderGraph::ResourceNode : public BindableRenderNode
	{
	public:
		/** Creates a new ResourceNode
		 *
		 * @param	name the name of the new ResourceNode */
		ResourceNode(const std::string& name) : BindableRenderNode(name) {};

		/** Class destructor */
		virtual ~ResourceNode() = default;

		/** Executes the current RenderNode */
		virtual void execute() override {};
	};


	/** Struct KDHitStackContent, its the data structure used in the
	 * RenderGraph sort algorithm for storing its state in a stack instead of
	 * using a recursive algorithm */
	struct RenderGraph::SortStackContent
	{
		/** The current RenderNode in with which we are working */
		RenderNode* node;

		/** If the parent RenderNode of the current one has been submitted or
		 * not */
		bool parentsSubmitted;
	};


	RenderGraph::RenderGraph()
	{
		auto resourcesNode = std::make_unique<ResourceNode>("resources");

		auto defaultFBIndex = resourcesNode->addBindable( std::shared_ptr<FrameBuffer>(&FrameBuffer::getDefaultFrameBuffer(), [](FrameBuffer*) {}) );
		resourcesNode->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("defaultFB", resourcesNode.get(), defaultFBIndex) );

		addNode( std::move(resourcesNode) );
	}


	bool RenderGraph::addNode(RenderNodeUPtr node)
	{
		if (node && std::none_of(
			mRenderNodes.begin(), mRenderNodes.end(),
			[&](const RenderNodeUPtr& node2) { return node2->getName() == node->getName(); }
		)) {
			mRenderNodes.emplace_back( std::move(node) );
			return true;
		}
		else {
			SOMBRA_ERROR_LOG << "There is another RenderNode with the same name";
			return false;
		}
	}


	bool RenderGraph::removeNode(RenderNode* node)
	{
		auto itRenderNode = std::find_if(mRenderNodes.begin(), mRenderNodes.end(), [&](const auto& node2) { return node2.get() == node; });
		if (itRenderNode != mRenderNodes.end()) {
			std::swap(*itRenderNode, mRenderNodes.back());
			mRenderNodes.pop_back();

			return true;
		}

		SOMBRA_ERROR_LOG << "There isn't any RenderNode with the same name";
		return false;
	}


	void RenderGraph::clearNodes()
	{
		mRenderNodes.clear();
	}


	RenderNode* RenderGraph::getNode(const std::string& nodeName) const
	{
		auto itRenderNode = std::find_if(
			mRenderNodes.begin(), mRenderNodes.end(),
			[&](const RenderNodeUPtr& node) { return node->getName() == nodeName; }
		);
		return (itRenderNode != mRenderNodes.end())? itRenderNode->get() : nullptr;
	}


	void RenderGraph::prepareGraph()
	{
		// Search the leaf nodes of the graph
		std::vector<bool> leafNodes(mRenderNodes.size(), true);
		for (const auto& node : mRenderNodes) {
			node->iterateInputs([&](RNodeInput& input) {
				RNodeOutput* connectedOutput = input.getConnectedOutput();
				if (connectedOutput && connectedOutput->getParentNode()) {
					auto itNode = std::find_if(
						mRenderNodes.begin(), mRenderNodes.end(),
						[&](const RenderNodeUPtr& node2) { return node2.get() == connectedOutput->getParentNode(); }
					);
					if (itNode != mRenderNodes.end()) {
						leafNodes[std::distance(mRenderNodes.begin(), itNode)] = false;
					}
					else {
						SOMBRA_WARN_LOG << "There is a RNodeOutput connected to the \"" << input.getName()
							<< "\" input of the \"" << node->getName() << "\" node that hasn't been added to the RenderGraph";
					}
				}
				else {
					SOMBRA_WARN_LOG << "There isn't any RNodeOutput connected to the \"" << input.getName()
						<< "\" input of the \"" << node->getName() << "\" node";
				}
			});
		}

		// Sort the Graph
		std::vector<RenderNodeUPtr> sortedNodes;
		sortedNodes.reserve(mRenderNodes.size());

		std::vector<SortStackContent> stack;
		stack.reserve(mRenderNodes.size());

		for (std::size_t i = 0; i < mRenderNodes.size(); ++i) { if (leafNodes[i]) {
			stack.push_back({ mRenderNodes[i].get(), false });
			while (!stack.empty()) {
				auto& top = stack.back();
				if (std::any_of(sortedNodes.begin(), sortedNodes.end(), [&](const RenderNodeUPtr& node2) { return node2.get() == top.node; })) {
					stack.pop_back();
				}
				else if (top.parentsSubmitted) {
					auto itNode = std::find_if(mRenderNodes.begin(), mRenderNodes.end(), [&](const RenderNodeUPtr& node2) { return node2.get() == top.node; });
					if (itNode != mRenderNodes.end()) {
						sortedNodes.emplace_back(std::move(*itNode));
					}
					stack.pop_back();
				}
				else {
					top.node->iterateInputs([&](RNodeInput& input) {
						RNodeOutput* connectedOutput = input.getConnectedOutput();
						if (connectedOutput && connectedOutput->getParentNode()) {
							stack.push_back({ connectedOutput->getParentNode(), false });
						}
					});
					top.parentsSubmitted = true;
				}
			}
		} }

		std::swap(sortedNodes, mRenderNodes);
	}


	void RenderGraph::execute()
	{
		for (auto& node : mRenderNodes) {
			node->execute();
		}
	}

}
