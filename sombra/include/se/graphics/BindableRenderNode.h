#ifndef BINDABLE_RENDER_NODE_H
#define BINDABLE_RENDER_NODE_H

#include "core/Bindable.h"
#include "RenderNode.h"

namespace se::graphics {

	class BindableRenderNode;
	template <typename T> class BindableRNodeInput;


	/**
	 * Class BindableRNodeConnector, it's the interface that an RNodeConnector
	 * must implement for accessing the Bindables inside a BindableRenderNode
	 */
	class BindableRNodeConnector
	{
	public:		// Nested types
		using BindableSPtr = std::shared_ptr<Bindable>;

	protected:	// Attributes
		/** The index of the Bindable resource inside @see mParentNode */
		std::size_t mBindableIndex;

	public:		// Functions
		/** Creates a new BindableRNodeConnector
		 *
		 * @param	bindableIndex the index of the Bindable inside the
		 *			parent RenderNode */
		BindableRNodeConnector(std::size_t bindableIndex) :
			mBindableIndex(bindableIndex) {};

		/** Class destructor */
		virtual ~BindableRNodeConnector() = default;

		/** The index of the Bindable resource inside @see mParentNode */
		std::size_t getBindableIndex() const { return mBindableIndex; };

		/** @return	a pointer to the Bindable resource */
		virtual BindableSPtr getBindable() const = 0;

		/** Notifies the BindableRNodeConnector of a change in the Bindable
		 * value */
		virtual void onBindableUpdate() = 0;
	};


	/**
	 * Class BindableRNodeOutput, it's a RNodeOutput with which a
	 * BindableRenderNode gives access to other BindableRenderNodes to one
	 * of its Bindable resources by connecting it to their BindableRNodeInputs
	 */
	template <typename T>
	class BindableRNodeOutput :
		public RNodeOutput, public BindableRNodeConnector
	{
	private:	// Attributes
		/** The connected BindableRNodeInputs to the current
		 * BindableRNodeOutput */
		std::vector<BindableRNodeInput<T>*> mConnectedInputs;

	public:
		/** Creates a new BindableRNodeOutput
		 *
		 * @param	name the name of the new BindableRNodeOutput
		 * @param	parentNode a pointer to the parent BindableRenderNode
		 *			of the new BindableRNodeOutput
		 * @param	bindableIndex the index of the Bindable resource inside
		 *			@see parentNode */
		BindableRNodeOutput(
			const std::string& name, BindableRenderNode* parentNode,
			std::size_t bindableIndex
		);

		/** @copydoc BindableRNodeConnector::getBindable() */
		virtual BindableSPtr getBindable() const override;

		/** @copydoc BindableRNodeConnector::onBindableUpdate() */
		virtual void onBindableUpdate() override;

		/** Adds the given BindableRNodeInput to @see mConnectedInputs */
		void addInput(BindableRNodeInput<T>* input);
	};


	/**
	 * Class BindableRNodeInput, it's a RNodeInput with which a
	 * BindableRenderNode can access to the Bindable resources of other
	 * BindableRenderNodes by connecting it to their BindableRNodeOutputs
	 */
	template <typename T>
	class BindableRNodeInput :
		public RNodeInput, public BindableRNodeConnector
	{
	public:		// Functions
		/** Creates a new BindableRNodeInput
		 *
		 * @param	name the name of the new BindableRNodeInput
		 * @param	parentNode a pointer to the parent BindableRenderNode
		 *			of the new BindableRNodeInput
		 * @param	bindableIndex the index where the connected Bindable
		 *			resource will be stored inside @see parentNode */
		BindableRNodeInput(
			const std::string& name, BindableRenderNode* parentNode,
			std::size_t bindableIndex
		);

		/** @copydoc RNodeInput::connect(RNodeOutput* output)
		 * @note It will also append the RNodeOutput's Bindable to the parent
		 * BindableRenderNode */
		virtual bool connect(RNodeOutput* output) override;

		/** @copydoc BindableRNodeConnector::getBindable() */
		virtual BindableSPtr getBindable() const override;

		/** @copydoc BindableRNodeConnector::onBindableUpdate() */
		virtual void onBindableUpdate() override;
	};


	/**
	 * Class BindableRenderNode, it's a RenderNode that holds Bindable resources
	 * that can be accessed with its Input and Output Connectors.
	 */
	class BindableRenderNode : public RenderNode, protected Bindable
	{
	protected:	// Nested types
		using BindableSPtr = std::shared_ptr<Bindable>;
		using BindableData = std::pair<BindableSPtr, bool>;

	private:	// Attributes
		/** All the Bindables added to the BindableRenderNode */
		std::vector<BindableData> mBindables;

	public:		// Functions
		/** Creates a new BindableRenderNode
		 *
		 * @param	name the name of the new BindableRenderNode */
		BindableRenderNode(const std::string& name) : RenderNode(name) {};

		/** Class destructor */
		virtual ~BindableRenderNode() = default;

		/** Adds the given Bindable to the BindableRenderNode
		 *
		 * @param	bindable a pointer to the new Bindable of the
		 *			BindableRenderNode
		 * @param	mustBind if the Bindable must be bound when the
		 *			BindableRenderNode @see bind function is called
		 * @return	the index of the new Bindable inside the
		 *			BindableRenderNode */
		std::size_t addBindable(
			BindableSPtr bindable = nullptr, bool mustBind = true
		);

		/** Returns the requested Bindable
		 *
		 * @param	bindableIndex the index of the Bindable to return inside the
		 *			BindableRenderNode
		 * @return	a pointer to the Bindable */
		BindableSPtr getBindable(std::size_t bindableIndex) const;

		/** Replaces the requested Bindable inside the BindableRenderNode
		 *
		 * @param	bindableIndex the index of the Bindable to replace
		 * @param	bindable the new Bindable */
		void setBindable(std::size_t bindableIndex, BindableSPtr bindable);
	protected:
		/** Binds the current object for using it in the following operations */
		virtual void bind() const override;

		/** Unbinds the current object so it can't be used in the following
		 * operations */
		virtual void unbind() const override;
	};

}

#include "BindableRenderNode.hpp"

#endif		// BINDABLE_RENDER_NODE_H
