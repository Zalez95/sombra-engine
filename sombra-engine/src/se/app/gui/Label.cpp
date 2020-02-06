#include "se/app/gui/Label.h"

namespace se::app {

	Label::Label(graphics::Layer2D* layer2D) :
		mRenderableText(mPosition, mSize), mLayer2D(layer2D)
	{
		mLayer2D->addRenderableText(&mRenderableText, mZIndex);
	}


	Label::~Label()
	{
		mLayer2D->removeRenderableText(&mRenderableText, mZIndex);
	}


	void Label::setPosition(const glm::vec2& position)
	{
		IComponent::setPosition(position);
		mRenderableText.setPosition(mPosition);
	}


	void Label::setSize(const glm::vec2& size)
	{
		IComponent::setSize(size);
		mRenderableText.setScale(mSize);
	}


	void Label::setZIndex(unsigned char zIndex)
	{
		mLayer2D->removeRenderableText(&mRenderableText, mZIndex);
		IComponent::setZIndex(zIndex);
		mLayer2D->addRenderableText(&mRenderableText, mZIndex);
	}


	void Label::setVisibility(bool isVisible)
	{
		bool wasVisible = mIsVisible;
		IComponent::setVisibility(isVisible);

		if (wasVisible && !mIsVisible) {
			mLayer2D->removeRenderableText(&mRenderableText, mZIndex);
		}
		else if (!wasVisible && mIsVisible) {
			mLayer2D->addRenderableText(&mRenderableText, mZIndex);
		}
	}


	void Label::setFont(FontSPtr font)
	{
		mRenderableText.setFont(font);
	}


	void Label::setCharacterScale(const glm::vec2& scale)
	{
		mRenderableText.setScale(scale);
	}


	void Label::setColor(const glm::vec4& color)
	{
		mRenderableText.setColor(color);
	}


	void Label::setText(const std::string& text)
	{
		mRenderableText.setText(text);
	}


	void Label::onHover(const MouseMoveEvent& /*event*/) {}


	void Label::onClick(const MouseButtonEvent& /*event*/) {}


	void Label::onRelease(const MouseButtonEvent& /*event*/) {}

}
