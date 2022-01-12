#include "se/graphics/BindableRenderNode.h"

namespace se::graphics {

	std::size_t BindableRenderNode::addBindable(const Context::BindableRef& bindable, bool mustBind)
	{
		mBindables.emplace(bindable, mustBind);
		return mBindables.size() - 1;
	}


	const Context::BindableRef& BindableRenderNode::getBindable(std::size_t bindableIndex) const
	{
		return mBindables[bindableIndex].first;
	}


	void BindableRenderNode::setBindable(std::size_t bindableIndex, const Context::BindableRef& bindable)
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


	void BindableRenderNode::bind(Context::Query& q) const
	{
		for (auto& [bindable, mustBind] : mBindables) {
			if (bindable && mustBind) {
				q.getBindable(bindable)->bind();
			}
		}
	}


	void BindableRenderNode::unbind(Context::Query& q) const
	{
		for (auto& [bindable, mustBind] : mBindables) {
			if (bindable && mustBind) {
				q.getBindable(bindable)->unbind();
			}
		}
	}

}
