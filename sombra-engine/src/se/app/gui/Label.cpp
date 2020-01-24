#include "se/app/gui/Label.h"

namespace se::app {

	Label::Label(graphics::Layer2D* layer2D) :
		mRenderableText(mPosition, mSize), mLayer2D(layer2D)
	{
		mLayer2D->addRenderableText(&mRenderableText);
	}


	Label::~Label()
	{
		mLayer2D->removeRenderableText(&mRenderableText);
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
		IComponent::setZIndex(zIndex);
		mRenderableText.setZIndex(mZIndex);
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

}
