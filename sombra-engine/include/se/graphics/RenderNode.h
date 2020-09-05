#ifndef RENDER_NODE_H
#define RENDER_NODE_H

#include <memory>
#include <string>
#include <vector>

namespace se::graphics {

	class RenderNode;


	/**
	 * Class RNodeConnector, it's the interface with which RenderNodes can be
	 * connected. It's used for implementing the edges of the RenderGraph.
	 */
	class RNodeConnector
	{
	private:	// Attributes
		/** The name used for identifying the current RNodeConnector */
		const std::string mName;

	public:		// Functions
		/** Creates a new RNodeConnector
		 *
		 * @param	name the name of the new RNodeConnector */
		RNodeConnector(const std::string& name) : mName(name) {};

		/** Class destructor */
		virtual ~RNodeConnector() = default;

		/** @return	the name of the current RNodeConnector */
		const std::string& getName() const { return mName; };

		/** @return	the RenderNode where the RNodeConnector resource is
		 *			located */
		virtual RenderNode* getParentNode() const = 0;
	};


	/**
	 * Class RNodeOutput, it's an outgoing RNodeConnector that can be connected
	 * to multiple RNodeInputs
	 */
	class RNodeOutput : public RNodeConnector
	{
	public:		// Functions
		/** Creates a new RNodeOutput
		 *
		 * @param	name the name of the new RNodeOutput */
		RNodeOutput(const std::string& name) : RNodeConnector(name) {};

		/** Class destructor */
		virtual ~RNodeOutput() = default;
	};


	/**
	 * Class RNodeInput, its an incoming RNodeConnector that can be connected
	 * to a single RNodeOutput
	 */
	class RNodeInput : public RNodeConnector
	{
	public:		// Functions
		/** Creates a new RNodeInput
		 *
		 * @param	name the name of the new RNodeInput */
		RNodeInput(const std::string& name) : RNodeConnector(name) {};

		/** Class destructor */
		virtual ~RNodeInput() = default;

		/** @return	the RNodeOutput connected to the current RNodeInput,
		 *			nullptr if there is no connection */
		virtual RNodeOutput* getConnectedOutput() const = 0;

		/** Connects the current RNodeInput to the given RNodeOutput
		 *
		 * @param	output a pointer to the RNodeOutput to connect
		 * @return	true if the RNodeOutput was connected succesfully, false
		 *			otherwise
		 * @note	you can't connect the same RNodeInput more than one time */
		virtual bool connect(RNodeOutput* output) = 0;
	};


	/**
	 * Class RenderNode, it's a node in the RenderGraph that can execute some
	 * function. It can be connected to other RenderNodes of the RenderGraph
	 * using its RNodeInputs and RNodeOutputs
	 */
	class RenderNode
	{
	protected:	// Nested types
		using InputUPtr = std::unique_ptr<RNodeInput>;
		using OutputUPtr = std::unique_ptr<RNodeOutput>;

	private:	// Attributes
		/** The name used for identifying the current RenderNode */
		const std::string mName;

		/** All the RNodeInputs of the RenderNode */
		std::vector<InputUPtr> mInputs;

		/** All the RNodeOutputs of the RenderNode */
		std::vector<OutputUPtr> mOutputs;

	public:		// Functions
		/** Creates a new RenderNode
		 *
		 * @param	name the name of the new RenderNode */
		RenderNode(const std::string& name) : mName(name) {};

		/** Class destructor */
		virtual ~RenderNode() = default;

		/** @return	the name of the current RenderNode */
		const std::string& getName() const { return mName; };

		/** Executes the current RenderNode */
		virtual void execute() = 0;

		/** Adds the given RNodeInput to the RenderNode
		 *
		 * @param	input a pointer to the new RNodeInput of the RenderNode
		 * @return	true if the RNodeInput was added succesfully, false
		 *			otherwise */
		bool addInput(InputUPtr input);

		/** Calls the given function for each RNodeInput of the RenderNode
		 *
		 * @param	callback the function to call */
		template <typename F>
		void iterateInputs(F callback);

		/** Searchs a RNodeInput with the same name than the given one
		 *
		 * @param	name the name of the RNodeInput to search
		 * @return	a pointer to the RNodeInput, nullptr if it wasn't found */
		RNodeInput* findInput(const std::string& name) const;

		/** Adds the given RNodeOutput to the RenderNode
		 *
		 * @param	output a pointer to the new RNodeOutput of the RenderNode
		 * @return	true if the RNodeOutput was added succesfully, false
		 *			otherwise */
		bool addOutput(OutputUPtr output);

		/** Calls the given function for each RNodeOutput of the RenderNode
		 *
		 * @param	callback the function to call */
		template <typename F>
		void iterateOutputs(F callback);

		/** Searchs a RNodeOutput with the same name than the given one
		 *
		 * @param	name the name of the RNodeOutput to search
		 * @return	a pointer to the RNodeOutput, nullptr if it wasn't found */
		RNodeOutput* findOutput(const std::string& name) const;
	};


	template <typename F>
	void RenderNode::iterateInputs(F callback)
	{
		for (auto& input : mInputs) {
			callback(*input);
		}
	}


	template <typename F>
	void RenderNode::iterateOutputs(F callback)
	{
		for (auto& output : mOutputs) {
			callback(*output);
		}
	}

}

#endif		// RENDER_NODE_H
