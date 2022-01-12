#ifndef BINDABLE_RENDER_NODE_H
#define BINDABLE_RENDER_NODE_H

#include "../utils/PackedVector.h"
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
		virtual Context::BindableRef getBindable() const = 0;

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
	public:		// Functions
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
		) : RNodeOutput(name, parentNode),
			BindableRNodeConnector(bindableIndex) {};

		/** @copydoc BindableRNodeConnector::getBindable() */
		virtual Context::BindableRef getBindable() const override;

		/** @return	a pointer to the typed Bindable resource */
		Context::TBindableRef<T> getTBindable() const
		{ return Context::TBindableRef<T>::from(getBindable()); };

		/** @copydoc BindableRNodeConnector::onBindableUpdate() */
		virtual void onBindableUpdate() override;
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
		) : RNodeInput(name, parentNode),
			BindableRNodeConnector(bindableIndex) {};

		/** @copydoc RNodeConnector::connect(RNodeConnector*)
		 * @note It will also append the RNodeConnector's Bindable to the parent
		 * BindableRenderNode */
		virtual bool connect(RNodeConnector* connector) override;

		/** @copydoc RNodeConnector::disconnect()
		 * @note It will also remove the RNodeConnector's Bindable from
		 * the parent BindableRenderNodes */
		virtual void disconnect() override;

		/** @copydoc BindableRNodeConnector::getBindable() */
		virtual Context::BindableRef getBindable() const override;

		/** @return	a pointer to the typed Bindable resource */
		Context::TBindableRef<T> getTBindable() const
		{ return Context::TBindableRef<T>::from(getBindable()); };

		/** @copydoc BindableRNodeConnector::onBindableUpdate() */
		virtual void onBindableUpdate() override;
	};


	/**
	 * Class BindableRenderNode, it's a RenderNode that holds Bindable resources
	 * that can be accessed with its Input and Output Connectors.
	 */
	class BindableRenderNode : public RenderNode
	{
	protected:	// Nested types
		using BindableData = std::pair<Context::BindableRef, bool>;

	private:	// Attributes
		/** All the Bindables added to the BindableRenderNode */
		utils::PackedVector<BindableData> mBindables;

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
		virtual std::size_t addBindable(
			const Context::BindableRef& bindable = {},
			bool mustBind = true
		);

		/** Returns the requested Bindable
		 *
		 * @param	bindableIndex the index of the Bindable to return inside the
		 *			BindableRenderNode
		 * @return	a pointer to the Bindable */
		virtual const Context::BindableRef& getBindable(
			std::size_t bindableIndex
		) const;

		/** Replaces the requested Bindable inside the BindableRenderNode
		 *
		 * @param	bindableIndex the index of the Bindable to replace
		 * @param	bindable the new Bindable */
		virtual void setBindable(
			std::size_t bindableIndex,
			const Context::BindableRef& bindable
		);

		/** Removes the requested Bindable from the BindableRenderNode
		 *
		 * @param	bindableIndex the index of the Bindable to remove */
		virtual void removeBindable(std::size_t bindableIndex);
	protected:
		/** Binds the current object for using it in the following operations
		 *
		 * @param	q the Context Query object used for accesing to the
		 *			Bindables */
		void bind(Context::Query& q) const;

		/** Unbinds the current object so it can't be used in the following
		 * operations
		 *
		 * @param	q the Context Query object used for accesing to the
		 *			Bindables */
		void unbind(Context::Query& q) const;
	};

}

#include "BindableRenderNode.hpp"

#endif		// BINDABLE_RENDER_NODE_H
