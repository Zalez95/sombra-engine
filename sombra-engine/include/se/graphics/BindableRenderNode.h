#ifndef BINDABLE_RENDER_NODE_H
#define BINDABLE_RENDER_NODE_H

#include "core/Bindable.h"
#include "RenderNode.h"

namespace se::graphics {

	class BindableRenderNode;


	/**
	 * Class BindableRNodeOutput, it's a RNodeOutput with which a
	 * BindableRenderNode gives access to other BindableRenderNodes to one
	 * of its Bindable resources by connecting it to their BindableRNodeInputs
	 */
	template <typename T>
	class BindableRNodeOutput : public RNodeOutput
	{
	private:	// Nested types
		using BindableSPtr = std::shared_ptr<Bindable>;

	private:	// Attributes
		/** A pointer to the BindableRenderNode where the BindableRNodeOutput
		 * is located */
		BindableRenderNode* mParentNode;

		/** The index of the Bindable resource inside @see mParentNode */
		std::size_t mBindableIndex;

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
		) : RNodeOutput(name), mParentNode(parentNode),
			mBindableIndex(bindableIndex) {};

		/** @return	the RenderNode where the RNodeOutput resource is located */
		virtual RenderNode* getParentNode() const override;

		/** @return	a pointer to the Bindable resource */
		BindableSPtr getBindable() const;
	};


	/**
	 * Class BindableRNodeInput, it's a RNodeInput with which a
	 * BindableRenderNode can access to the Bindable resources of other
	 * BindableRenderNodes by connecting it to their BindableRNodeOutputs
	 */
	template <typename T>
	class BindableRNodeInput : public RNodeInput
	{
	private:	// Attributes
		/** A pointer to the parent BindableRenderNode of the current
		 * BindableRNodeInput */
		BindableRenderNode* mParentNode;

		/** The index where the connected Bindable resource will be stored
		 * inside @see mParentNode */
		std::size_t mBindableIndex;

		/** The BindableRNodeOutput connected to the current
		 * BindableRNodeInput */
		BindableRNodeOutput<T>* mConnectedOutput;

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
		) : RNodeInput(name), mParentNode(parentNode),
			mBindableIndex(bindableIndex), mConnectedOutput(nullptr) {};

		/** @return	the RenderNode where the RNodeOutput resource is located */
		virtual RenderNode* getParentNode() const override;

		/** @return	the RNodeOutput connected to the current RNodeInput,
		 *			nullptr if there is no connection */
		virtual RNodeOutput* getConnectedOutput() const override;

		/** Connects the current BindableRNodeInput to the given RNodeOutput.
		 * It will also append the RNodeOutput's Bindable to the parent
		 * BindableRenderNode
		 *
		 * @param	output a pointer to the RNodeOutput to connect
		 * @return	true if the RNodeOutput was connected succesfully, false
		 *			otherwise
		 * @note	you can't connect the same RNodeInput more than one time */
		virtual bool connect(RNodeOutput* output) override;
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
