#ifndef BINDABLE_RENDER_NODE_HPP
#define BINDABLE_RENDER_NODE_HPP

#include "../utils/Log.h"

namespace se::graphics {

	template <typename T>
	typename BindableRNodeOutput<T>::BindableSPtr BindableRNodeOutput<T>::getBindable() const
	{
		return static_cast<const BindableRenderNode*>(mParentNode)->getBindable(mBindableIndex);
	}


	template <typename T>
	void BindableRNodeOutput<T>::onBindableUpdate()
	{
		for (RNodeInput* input : mConnectedInputs) {
			static_cast<BindableRNodeInput<T>*>(input)->onBindableUpdate();
		}
	}


	template <typename T>
	bool BindableRNodeInput<T>::connect(RNodeConnector* connector)
	{
		if (!connector) {
			SOMBRA_ERROR_LOG << "Trying to attach NULL connector to " << mParentNode->getName() << "[" << mName << "]";
			return false;
		}
		else if (auto output = dynamic_cast<BindableRNodeOutput<T>*>(connector)) {
			if (RNodeInput::connect(connector)) {
				auto parent = static_cast<BindableRenderNode*>(mParentNode);
				parent->setBindable(mBindableIndex, output->getBindable());
				return true;
			}
			else {
				return false;
			}
		}
		else {
			SOMBRA_ERROR_LOG << "Trying to attach " << connector->getParentNode()->getName() << "[" << connector->getName() << "]"
				<< " with invalid type to " << mParentNode->getName() << "[" << mName << "]";
			return false;
		}
	}


	template <typename T>
	void BindableRNodeInput<T>::disconnect()
	{
		if (mConnectedOutput) {
			auto parent = static_cast<BindableRenderNode*>(mParentNode);
			parent->setBindable(mBindableIndex, nullptr);
		}

		RNodeInput::disconnect();
	}


	template <typename T>
	typename BindableRNodeInput<T>::BindableSPtr BindableRNodeInput<T>::getBindable() const
	{
		return static_cast<const BindableRenderNode*>(mParentNode)->getBindable(mBindableIndex);
	}


	template <typename T>
	void BindableRNodeInput<T>::onBindableUpdate()
	{
		auto parent = static_cast<BindableRenderNode*>(mParentNode);
		auto output = static_cast<BindableRNodeOutput<T>*>(mConnectedOutput);
		parent->setBindable(mBindableIndex, output->getBindable());
	}

}

#endif		// BINDABLE_RENDER_NODE_HPP
