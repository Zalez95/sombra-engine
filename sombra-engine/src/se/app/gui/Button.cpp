#include "se/app/gui/Button.h"
#include "se/app/gui/Label.h"
#include "se/window/MouseButtonCodes.h"

namespace se::app {

	Button::Button(graphics::Layer2D* layer2D, IBoundsIPtr bounds) :
		mLayer2D(layer2D), mBounds(std::move(bounds)), mRenderable2D(mPosition, mSize),
		mIsOver(false), mIsPressed(false), mLabel(nullptr)
	{
		mBounds->setPosition(mSize);
		mBounds->setPosition(mPosition);
		setVisibility(true);
	}


	Button::~Button()
	{
		setVisibility(false);
	}


	void Button::setPosition(const glm::vec2& position)
	{
		IComponent::setPosition(position);
		mBounds->setPosition(mPosition + mSize / 2.0f);
		mRenderable2D.setPosition(mPosition);
		mLabel->setPosition(mPosition);
	}


	void Button::setSize(const glm::vec2& size)
	{
		IComponent::setSize(size);
		mBounds->setSize(mSize);
		mRenderable2D.setSize(mSize);
		mLabel->setSize(mSize);
	}


	void Button::setZIndex(unsigned char zIndex)
	{
		mLayer2D->removeRenderable2D(&mRenderable2D, mZIndex);
		IComponent::setZIndex(zIndex);
		mLayer2D->addRenderable2D(&mRenderable2D, mZIndex);

		if (mLabel) {
			mLabel->setZIndex(mZIndex + 1);
		}
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

		if (mLabel) {
			mLabel->setVisibility(mIsVisible);
		}
	}


	void Button::setColor(const glm::vec4& color)
	{
		mRenderable2D.setColor(color);
	}


	void Button::setLabel(Label* label)
	{
		mLabel = label;
		mLabel->setSize(mSize);
		mLabel->setPosition(mPosition);
		mLabel->setZIndex(mZIndex + 1);
		mLabel->setVisibility(mIsVisible);
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
