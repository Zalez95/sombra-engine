#include <algorithm>
#include "se/app/gui/ComposedComponent.h"

namespace se::app {

	void ComposedComponent::onHover(const MouseMoveEvent& event)
	{
		for (auto& child : mChildren) {
			child->onHover(event);
		}
	}


	void ComposedComponent::onClick(const MouseButtonEvent& event)
	{
		for (auto& child : mChildren) {
			child->onClick(event);
		}
	}


	void ComposedComponent::onRelease(const MouseButtonEvent& event)
	{
		for (auto& child : mChildren) {
			child->onRelease(event);
		}
	}


	void ComposedComponent::add(IComponent* child)
	{
		mChildren.push_back(child);
	}


	void ComposedComponent::remove(IComponent* child)
	{
		mChildren.erase(std::remove(mChildren.begin(), mChildren.end(), child), mChildren.end());
	}

}
