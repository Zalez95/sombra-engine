#include "se/graphics/BindableRenderNode.h"

namespace se::graphics {

	std::size_t BindableRenderNode::addBindable(BindableSPtr bindable)
	{
		mBindables.push_back(bindable);
		return mBindables.size() - 1;
	}


	BindableRenderNode::BindableSPtr BindableRenderNode::getBindable(std::size_t bindableIndex) const
	{
		return mBindables[bindableIndex];
	}


	void BindableRenderNode::setBindable(std::size_t bindableIndex, BindableSPtr bindable)
	{
		mBindables[bindableIndex] = bindable;
	}


	void BindableRenderNode::bind() const
	{
		for (const BindableSPtr& bindable : mBindables) {
			bindable->bind();
		}
	}


	void BindableRenderNode::unbind() const
	{
		for (const BindableSPtr& bindable : mBindables) {
			bindable->unbind();
		}
	}

}
