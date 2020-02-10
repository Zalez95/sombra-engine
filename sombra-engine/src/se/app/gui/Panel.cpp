#include "se/app/gui/Panel.h"

namespace se::app {

	Panel::Panel(graphics::Layer2D* layer2D) : mLayer2D(layer2D), mRenderable2D(mPosition, mSize)
	{
		setVisibility(true);
	}


	Panel::~Panel()
	{
		setVisibility(false);
	}


	void Panel::setPosition(const glm::vec2& position)
	{
		ComposedComponent::setPosition(position);
		mRenderable2D.setPosition(mPosition);
	}


	void Panel::setSize(const glm::vec2& size)
	{
		ComposedComponent::setSize(size);
		mRenderable2D.setSize(mSize);
	}


	void Panel::setZIndex(unsigned char zIndex)
	{
		mLayer2D->removeRenderable2D(&mRenderable2D, mZIndex);
		ComposedComponent::setZIndex(zIndex);
		mLayer2D->addRenderable2D(&mRenderable2D, mZIndex);
	}


	void Panel::setVisibility(bool isVisible)
	{
		bool wasVisible = mIsVisible;
		ComposedComponent::setVisibility(isVisible);

		if (wasVisible && !mIsVisible) {
			mLayer2D->removeRenderable2D(&mRenderable2D, mZIndex);
		}
		else if (!wasVisible && mIsVisible) {
			mLayer2D->addRenderable2D(&mRenderable2D, mZIndex);
		}
	}


	void Panel::setColor(const glm::vec4& color)
	{
		mRenderable2D.setColor(color);
	}

}
