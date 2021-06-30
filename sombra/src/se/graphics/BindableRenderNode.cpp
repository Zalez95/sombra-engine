#include "se/graphics/BindableRenderNode.h"

namespace se::graphics {

	std::size_t BindableRenderNode::addBindable(BindableSPtr bindable, bool mustBind)
	{
		mBindables.emplace(bindable, mustBind);
		return mBindables.size() - 1;
	}


	BindableRenderNode::BindableSPtr BindableRenderNode::getBindable(std::size_t bindableIndex) const
	{
		return mBindables[bindableIndex].first;
	}


	void BindableRenderNode::setBindable(std::size_t bindableIndex, const BindableSPtr& bindable)
	{
		mBindables[bindableIndex].first = bindable;
		iterateOutputs([&](RNodeOutput& output) {
			if (auto outputPtr = dynamic_cast<BindableRNodeConnector*>(&output)) {
				if (outputPtr->getBindableIndex() == bindableIndex) {
					outputPtr->onBindableUpdate();
				}
			}
		});
	}


	void BindableRenderNode::removeBindable(std::size_t bindableIndex)
	{
		mBindables.erase(mBindables.begin().setIndex(bindableIndex));
	}


	void BindableRenderNode::bind() const
	{
		for (auto& [bindable, mustBind] : mBindables) {
			if (bindable && mustBind) {
				bindable->bind();
			}
		}
	}


	void BindableRenderNode::unbind() const
	{
		for (auto& [bindable, mustBind] : mBindables) {
			if (bindable && mustBind) {
				bindable->unbind();
			}
		}
	}

}
