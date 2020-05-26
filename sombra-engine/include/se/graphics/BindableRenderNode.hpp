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
	typename BindableRNodeOutput<T>::BindableSPtr BindableRNodeOutput<T>::getBindable() const
	{
		return mParentNode->getBindable(mBindableIndex);
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
			mParentNode->setBindable(mBindableIndex, mConnectedOutput->getBindable());
			return true;
		}
		else {
			SOMBRA_ERROR_LOG << "Invalid type";
			return false;
		}
	}

}

#endif		// BINDABLE_RENDER_NODE_HPP
