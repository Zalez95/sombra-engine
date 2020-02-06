#include "se/app/gui/Button.h"
#include "se/window/MouseButtonCodes.h"

namespace se::app {

	Button::Button(IBoundsIPtr bounds, graphics::Layer2D* layer2D) :
		mBounds(std::move(bounds)), mRenderable2D(mPosition, mSize), mLayer2D(layer2D),
		mIsOver(false), mIsPressed(false)
	{
		mBounds->setPosition(mSize);
		mBounds->setPosition(mPosition);
		mLayer2D->addRenderable2D(&mRenderable2D, mZIndex);
	}


	Button::~Button()
	{
		mLayer2D->removeRenderable2D(&mRenderable2D, mZIndex);
	}


	void Button::setPosition(const glm::vec2& position)
	{
		IComponent::setPosition(position);
		mBounds->setPosition(mPosition + mSize / 2.0f);
		mRenderable2D.setPosition(mPosition);
	}


	void Button::setSize(const glm::vec2& size)
	{
		IComponent::setSize(size);
		mBounds->setSize(mSize);
		mRenderable2D.setScale(mSize);
	}


	void Button::setZIndex(unsigned char zIndex)
	{
		mLayer2D->removeRenderable2D(&mRenderable2D, mZIndex);
		IComponent::setZIndex(zIndex);
		mLayer2D->addRenderable2D(&mRenderable2D, mZIndex);
	}


	void Button::setVisibility(bool isVisible)
	{
		bool wasVisible = mIsVisible;
		IComponent::setVisibility(isVisible);

		if (wasVisible && !mIsVisible) {
			mLayer2D->removeRenderable2D(&mRenderable2D, mZIndex);
		}
		else if (!wasVisible && mIsVisible) {
			mLayer2D->addRenderable2D(&mRenderable2D, mZIndex);
		}
	}


	void Button::setColor(const glm::vec4& color)
	{
		mRenderable2D.setColor(color);
	}


	void Button::setAction(const std::function<void()>& action)
	{
		mAction = action;
	}


	void Button::onHover(const MouseMoveEvent& event)
	{
		glm::vec2 mousePosition(static_cast<float>(event.getX()), static_cast<float>(event.getY()));
		mIsOver = mIsVisible && mBounds->contains(mousePosition);
	}


	void Button::onClick(const MouseButtonEvent& event)
	{
		if (mIsOver && (event.getKeyCode() == SE_MOUSE_BUTTON_1)) {
			mIsPressed = true;
		}
	}


	void Button::onRelease(const MouseButtonEvent& event)
	{
		if (mIsPressed && (event.getKeyCode() == SE_MOUSE_BUTTON_1)) {
			mAction();
			mIsPressed = false;
		}
	}

}
