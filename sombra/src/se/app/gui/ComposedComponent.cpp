#include <algorithm>
#include "se/app/gui/ComposedComponent.h"

namespace se::app {

	void ComposedComponent::setPosition(const glm::vec2& position)
	{
		IComponent::setPosition(position);
		for (auto& childData : mChildren) {
			childData.child->setPosition( calculateChildPosition(*childData.child, childData.anchor) );
		}
	}


	void ComposedComponent::setSize(const glm::vec2& size)
	{
		IComponent::setSize(size);
		for (auto& childData : mChildren) {
			childData.child->setSize( calculateChildSize(childData.proportions) );
			childData.child->setPosition( calculateChildPosition(*childData.child, childData.anchor) );
		}
	}


	void ComposedComponent::setZIndex(unsigned char zIndex)
	{
		IComponent::setZIndex(zIndex);
		for (auto& childData : mChildren) {
			childData.child->setZIndex(mZIndex + 1);
		}
	}


	void ComposedComponent::setVisibility(bool isVisible)
	{
		IComponent::setVisibility(isVisible);
		for (auto& childData : mChildren) {
			childData.child->setVisibility(mIsVisible);
		}
	}


	void ComposedComponent::onHover(const MouseMoveEvent& event)
	{
		for (auto& childData : mChildren) {
			childData.child->onHover(event);
		}
	}


	void ComposedComponent::onClick(const MouseButtonEvent& event)
	{
		for (auto& childData : mChildren) {
			childData.child->onClick(event);
		}
	}


	void ComposedComponent::onRelease(const MouseButtonEvent& event)
	{
		for (auto& childData : mChildren) {
			childData.child->onRelease(event);
		}
	}


	void ComposedComponent::add(IComponent* child, const Anchor& anchor, const Proportions& proportions)
	{
		child->setZIndex(mZIndex + 1);
		child->setSize( calculateChildSize(proportions) );
		child->setPosition( calculateChildPosition(*child, anchor) );
		mChildren.push_back({ child, anchor, proportions });
	}


	void ComposedComponent::remove(IComponent* child)
	{
		mChildren.erase(
			std::remove_if(mChildren.begin(), mChildren.end(), [&](const ChildData& cd) { return cd.child == child; }),
			mChildren.end()
		);
	}

// Private functions
	glm::vec2 ComposedComponent::calculateChildPosition(const IComponent& child, const Anchor& anchor) const
	{
		glm::vec2 originLocation(0.0f);
		switch (anchor.origin) {
			case Anchor::Origin::TopRight:
				originLocation.x = -child.getSize().x;
				break;
			case Anchor::Origin::BottomLeft:
				originLocation.y = -child.getSize().y;
				break;
			case Anchor::Origin::BottomRight:
				originLocation = -child.getSize();
				break;
			case Anchor::Origin::Center:
				originLocation = -0.5f * child.getSize();
				break;
			default:
				break;
		}

		return mPosition + mSize * anchor.relativePosition + anchor.offset + originLocation;
	}


	glm::vec2 ComposedComponent::calculateChildSize(const Proportions& proportions) const
	{
		glm::vec2 size = glm::clamp(mSize * proportions.relativeSize, proportions.minimumSize, proportions.maximumSize);

		if (proportions.preserveAspectRatio) {
			// TODO:
		}

		return size;
	}

}
