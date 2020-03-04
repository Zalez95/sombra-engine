#include "se/app/gui/Button.h"
#include "se/app/gui/Label.h"
#include "se/window/MouseButtonCodes.h"

namespace se::app {

	Button::Button(graphics::Layer2D* layer2D, IBoundsUPtr bounds) :
		mLayer2D(layer2D), mBounds(std::move(bounds)), mRenderable2D(mPosition, mSize),
		mIsOver(false), mIsPressed(false), mLabel(nullptr), mLabelScale(1.0f)
	{
		mBounds->setPosition(mSize);
		mBounds->setPosition(mPosition);
		setVisibility(true);
	}


	Button::Button(const Button& other) :
		mLayer2D(other.mLayer2D),
		mBounds(IBoundsUPtr(other.mBounds->clone())),
		mRenderable2D(other.mRenderable2D),
		mIsOver(other.mIsOver),
		mIsPressed(other.mIsPressed),
		mLabel(other.mLabel),
		mLabelScale(other.mLabelScale),
		mAction(other.mAction) {}


	Button::~Button()
	{
		setVisibility(false);
	}


	Button& Button::operator=(const Button& other)
	{
		mLayer2D = other.mLayer2D;
		mBounds = IBoundsUPtr(other.mBounds->clone());
		mRenderable2D = other.mRenderable2D;
		mIsOver = other.mIsOver;
		mIsPressed = other.mIsPressed;
		mLabel = other.mLabel;
		mLabelScale = other.mLabelScale;
		mAction = other.mAction;

		return *this;
	}


	void Button::setPosition(const glm::vec2& position)
	{
		IComponent::setPosition(position);
		mBounds->setPosition(mPosition + mSize / 2.0f);
		mRenderable2D.setPosition(mPosition);

		if (mLabel) {
			mLabel->setPosition(mPosition + (mSize - mLabel->getSize()) / 2.0f);
		}
	}


	void Button::setSize(const glm::vec2& size)
	{
		IComponent::setSize(size);
		mBounds->setSize(mSize);
		mRenderable2D.setSize(mSize);

		if (mLabel) {
			mLabel->setSize(mLabelScale * mSize);
		}
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


	void Button::setLabel(Label* label, const glm::vec2& labelScale)
	{
		mLabel = label;
		mLabelScale = labelScale;

		mLabel->setSize(mLabelScale * mSize);
		mLabel->setPosition(mPosition + (mSize - mLabel->getSize()) / 2.0f);
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
		if (mIsOver && (event.getButtonCode() == SE_MOUSE_BUTTON_LEFT)) {
			mIsPressed = true;
		}
	}


	void Button::onRelease(const MouseButtonEvent& event)
	{
		if (mIsPressed && (event.getButtonCode() == SE_MOUSE_BUTTON_LEFT)) {
			mAction();
			mIsPressed = false;
		}
	}

}
