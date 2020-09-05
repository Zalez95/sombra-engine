#ifndef BINDABLE_RENDER_NODE_HPP
#define BINDABLE_RENDER_NODE_HPP

#include "../utils/Log.h"

namespace se::graphics {

	template <typename T>
	RenderNode* BindableRNodeOutput<T>::getParentNode() const
	{
		return mParentNode;
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
	RenderNode* BindableRNodeInput<T>::getParentNode() const
	{
		return mParentNode;
	}


	template <typename T>
	RNodeOutput* BindableRNodeInput<T>::getConnectedOutput() const
	{
		return mConnectedOutput;
	}


	template <typename T>
	bool BindableRNodeInput<T>::connect(RNodeOutput* output)
	{
		if (auto tmp = dynamic_cast<BindableRNodeOutput<T>*>(output)) {
			if (mConnectedOutput) {
				SOMBRA_ERROR_LOG << "Can't replace older connection";
				return false;
			}

			mConnectedOutput = tmp;
			mConnectedOutput->addInput(this);
			mParentNode->setBindable(mBindableIndex, mConnectedOutput->getBindable());
			return true;
		}
		else {
			SOMBRA_ERROR_LOG << "Invalid type";
			return false;
		}
	}


	template <typename T>
	void BindableRNodeInput<T>::onBindableUpdate()
	{
		mParentNode->setBindable(mBindableIndex, mConnectedOutput->getBindable());
	}

}

#endif		// BINDABLE_RENDER_NODE_HPP
