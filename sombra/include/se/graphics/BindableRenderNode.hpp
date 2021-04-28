#ifndef BINDABLE_RENDER_NODE_HPP
#define BINDABLE_RENDER_NODE_HPP

#include "../utils/Log.h"

namespace se::graphics {

	template <typename T>
	BindableRNodeOutput<T>::BindableRNodeOutput(
		const std::string& name, BindableRenderNode* parentNode,
		std::size_t bindableIndex
	) : RNodeOutput(name, parentNode),
		BindableRNodeConnector(bindableIndex) {}


	template <typename T>
	typename BindableRNodeOutput<T>::BindableSPtr BindableRNodeOutput<T>::getBindable() const
	{
		return static_cast<const BindableRenderNode*>(mParentNode)->getBindable(mBindableIndex);
	}


	template <typename T>
	void BindableRNodeOutput<T>::onBindableUpdate()
	{
		for (auto input : mConnectedInputs) {
			input->onBindableUpdate();
		}
	}


	template <typename T>
	void BindableRNodeOutput<T>::addInput(BindableRNodeInput<T>* input)
	{
		mConnectedInputs.push_back(input);
	}


	template <typename T>
	BindableRNodeInput<T>::BindableRNodeInput(
		const std::string& name, BindableRenderNode* parentNode,
		std::size_t bindableIndex
	) : RNodeInput(name, parentNode),
		BindableRNodeConnector(bindableIndex) {}


	template <typename T>
	bool BindableRNodeInput<T>::connect(RNodeOutput* output)
	{
		if (auto tOutput = dynamic_cast<BindableRNodeOutput<T>*>(output)) {
			if (RNodeInput::connect(tOutput)) {
				auto parent = static_cast<BindableRenderNode*>(mParentNode);

				tOutput->addInput(this);
				parent->setBindable(mBindableIndex, tOutput->getBindable());
				return true;
			}
			else {
				return false;
			}
		}
		else {
			SOMBRA_ERROR_LOG << "Trying to attach " << output->getParentNode()->getName() << "[" << output->getName() << "]"
				<< " with invalid type to " << mParentNode->getName() << "[" << mName << "]";
			return false;
		}
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
