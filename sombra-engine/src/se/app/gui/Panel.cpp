#include "se/app/gui/Panel.h"

namespace se::app {

	Panel::Panel(graphics::Layer2D* layer2D) :
		mRenderable2D(mPosition, mSize), mLayer2D(layer2D)
	{
		mLayer2D->addRenderable2D(&mRenderable2D);
	}


	Panel::~Panel()
	{
		mLayer2D->removeRenderable2D(&mRenderable2D);
	}


	void Panel::setPosition(const glm::vec2& position)
	{
		IComponent::setPosition(position);
		mRenderable2D.setPosition(mPosition);
	}


	void Panel::setSize(const glm::vec2& size)
	{
		IComponent::setSize(size);
		mRenderable2D.setScale(mSize);
	}


	void Panel::setZIndex(unsigned char zIndex)
	{
		IComponent::setZIndex(zIndex);
		mRenderable2D.setZIndex(mZIndex);
	}

}
