#ifndef RENDER_NODE_H
#define RENDER_NODE_H

#include <string>
#include "Context.h"

namespace se::graphics {

	class RenderNode;
	class RNodeInput;


	/**
	 * Class RNodeConnector, it's the interface with which RenderNodes can be
	 * connected. It's used for implementing the edges of the RenderGraph.
	 */
	class RNodeConnector
	{
	protected:	// Attributes
		/** The name used for identifying the current RNodeConnector */
		const std::string mName;

		/** A pointer to the RenderNode where the RNodeConnector is located */
		RenderNode* mParentNode;

	public:		// Functions
		/** Creates a new RNodeConnector
		 *
		 * @param	name the name of the new RNodeConnector
		 * @param	parentNode a pointer to the parent RenderNode where the
		 *			RNodeConnector is located */
		RNodeConnector(const std::string& name, RenderNode* parentNode) :
			mName(name), mParentNode(parentNode) {};

		/** Class destructor */
		virtual ~RNodeConnector() = default;

		/** @return	the name of the current RNodeConnector */
		const std::string& getName() const { return mName; };

		/** @return	the RenderNode where the RNodeConnector is located */
		RenderNode* getParentNode() const { return mParentNode; };

		/** Connects the current RNodeConnector to the given RNodeConnector
		 *
		 * @param	connector a pointer to the RNodeConnector to connect
		 * @return	true if the RNodeConnector was connected succesfully, false
		 *			otherwise */
		virtual bool connect(RNodeConnector* /*connector*/) { return true; };

		/** Disconnects the current RNodeConnector from any other
		 * RNodeConnector that is connected */
		virtual void disconnect() {};
	};


	/**
	 * Class RNodeOutput, it's an outgoing RNodeConnector that can be connected
	 * to multiple RNodeInputs
	 */
	class RNodeOutput : public RNodeConnector
	{
	protected:	// Attributes
		friend class RNodeInput;

		/** The connected RNodeInputs to the current RNodeOutput */
		std::vector<RNodeInput*> mConnectedInputs;

	public:		// Functions
		/** Creates a new RNodeOutput
		 *
		 * @param	name the name of the new RNodeOutput
		 * @param	parentNode a pointer to the parent RenderNode where the
		 *			RNodeOutput is located */
		RNodeOutput(const std::string& name, RenderNode* parentNode) :
			RNodeConnector(name, parentNode) {};

		/** Class destructor */
		virtual ~RNodeOutput() = default;

		/** @return	true if the RenderNode has any connections, false
		 *			otherwise */
		bool hasConnections() const { return !mConnectedInputs.empty(); };

		/** @copydoc RNodeConnector::connect(RNodeConnector*) */
		virtual bool connect(RNodeConnector* connector) override;

		/** @copydoc RNodeConnector::disconnect() */
		virtual void disconnect() override;
	protected:
		/** Adds the given RNodeInput to @see mConnectedInputs */
		void addInput(RNodeInput* input);

		/** Removes the given RNodeInput from @see mConnectedInputs */
		void removeInput(RNodeInput* input);
	};


	/**
	 * Class RNodeInput, it's an incoming RNodeConnector that can be connected
	 * to a single RNodeOutput
	 */
	class RNodeInput : public RNodeConnector
	{
	protected:	// Attributes
		/** The RNodeOutput connected to the current RNodeInput */
		RNodeOutput* mConnectedOutput;

	public:		// Functions
		/** Creates a new RNodeInput
		 *
		 * @param	name the name of the new RNodeInput
		 * @param	parentNode a pointer to the parent RenderNode where the
		 *			RNodeInput is located */
		RNodeInput(const std::string& name, RenderNode* parentNode) :
			RNodeConnector(name, parentNode), mConnectedOutput(nullptr) {};

		/** Class destructor */
		virtual ~RNodeInput() = default;

		/** @return	the RNodeOutput connected to the current RNodeInput,
		 *			nullptr if there is no connection */
		RNodeOutput* getConnectedOutput() const { return mConnectedOutput; };

		/** @copydoc RNodeConnector::connect(RNodeConnector*)
		 * @note	you can't connect the same RNodeInput more than one time */
		virtual bool connect(RNodeConnector* connector) override;

		/** @copydoc RNodeConnector::disconnect() */
		virtual void disconnect() override;
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

		/** Executes the current RenderNode
		 *
		 * @param	q the Context Query object used for accesing to the
		 *			Bindables */
		virtual void execute(Context::Query& q) = 0;

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
		void iterateInputs(F&& callback);

		/** Searchs a RNodeInput with the same name than the given one
		 *
		 * @param	name the name of the RNodeInput to search
		 * @return	a pointer to the RNodeInput, nullptr if it wasn't found */
		RNodeInput* findInput(const std::string& name) const;

		/** Removes the input with the same name than the given one
		 *
		 * @param	input a pointer to the RNodeInput to remove
		 * @return	true if the RNodeInput was removed succesfully, false
		 *			otherwise */
		bool removeInput(RNodeInput* input);

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
		void iterateOutputs(F&& callback);

		/** Searchs a RNodeOutput with the same name than the given one
		 *
		 * @param	name the name of the RNodeOutput to search
		 * @return	a pointer to the RNodeOutput, nullptr if it wasn't found */
		RNodeOutput* findOutput(const std::string& name) const;

		/** Removes the output with the same name than the given one
		 *
		 * @param	output a pointer to the RNodeOutput to remove
		 * @return	true if the RNodeOutput was removed succesfully, false
		 *			otherwise */
		bool removeOutput(RNodeOutput* output);

		/** Disconnects all the RNodeConnectors from the RenderNode */
		void disconnect();
	};


	template <typename F>
	void RenderNode::iterateInputs(F&& callback)
	{
		for (auto& input : mInputs) {
			callback(*input);
		}
	}


	template <typename F>
	void RenderNode::iterateOutputs(F&& callback)
	{
		for (auto& output : mOutputs) {
			callback(*output);
		}
	}

}

#endif		// RENDER_NODE_H
