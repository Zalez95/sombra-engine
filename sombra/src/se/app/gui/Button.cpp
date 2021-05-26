#include "se/app/Repository.h"
#include "se/window/MouseButtonCodes.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/app/gui/Button.h"
#include "se/app/gui/Label.h"
#include "se/app/gui/GUIManager.h"
#include "se/app/Application.h"

namespace se::app {

	Button::Button(GUIManager* guiManager, IBoundsUPtr bounds) :
		mGUIManager(guiManager), mBounds(std::move(bounds)), mSprite(mPosition, mSize),
		mIsOver(false), mIsPressed(false), mLabel(nullptr), mLabelScale(1.0f)
	{
		mBounds->setPosition(mSize);
		mBounds->setPosition(mPosition);

		mSprite.setZIndex(mZIndex);
		auto& application = mGUIManager->getApplication();
		auto technique2D = application.getRepository().find<std::string, graphics::Technique>("technique2D");
		if (technique2D) {
			mSprite.addTechnique(technique2D);
		}

		mIsVisible = false;
		setVisibility(true);
	}


	Button::Button(const Button& other) :
		IComponent(other),
		mGUIManager(other.mGUIManager),
		mBounds(IBoundsUPtr(other.mBounds->clone())),
		mSprite(other.mSprite),
		mIsOver(other.mIsOver),
		mIsPressed(other.mIsPressed),
		mLabel(other.mLabel),
		mLabelScale(other.mLabelScale),
		mAction(other.mAction)
	{
		mIsVisible = false;
		setVisibility(other.mIsVisible);
	}


	Button::~Button()
	{
		setVisibility(false);
	}


	Button& Button::operator=(const Button& other)
	{
		IComponent::operator=(other);
		mGUIManager = other.mGUIManager;
		mBounds = IBoundsUPtr(other.mBounds->clone());
		mSprite = other.mSprite;
		mIsOver = other.mIsOver;
		mIsPressed = other.mIsPressed;
		mLabel = other.mLabel;
		mLabelScale = other.mLabelScale;
		mAction = other.mAction;

		mIsVisible = false;
		setVisibility(other.mIsVisible);

		return *this;
	}


	void Button::setPosition(const glm::vec2& position)
	{
		IComponent::setPosition(position);
		mBounds->setPosition(mPosition + mSize / 2.0f);
		mSprite.setPosition(mPosition);

		if (mLabel) {
			mLabel->setPosition(mPosition + (mSize - mLabel->getSize()) / 2.0f);
		}
	}


	void Button::setSize(const glm::vec2& size)
	{
		IComponent::setSize(size);
		mBounds->setSize(mSize);
		mSprite.setSize(mSize);

		if (mLabel) {
			mLabel->setSize(mLabelScale * mSize);
		}
	}


	void Button::setZIndex(unsigned char zIndex)
	{
		IComponent::setZIndex(zIndex);
		mSprite.setZIndex(mZIndex);

		if (mLabel) {
			mLabel->setZIndex(mZIndex + 1);
		}
	}


	void Button::setVisibility(bool isVisible)
	{
		bool wasVisible = mIsVisible;
		IComponent::setVisibility(isVisible);

		auto& application = mGUIManager->getApplication();
		if (wasVisible && !mIsVisible) {
			application.getExternalTools().graphicsEngine->removeRenderable(&mSprite);
		}
		else if (!wasVisible && mIsVisible) {
			application.getExternalTools().graphicsEngine->addRenderable(&mSprite);
		}

		if (mLabel) {
			mLabel->setVisibility(mIsVisible);
		}
	}


	void Button::setColor(const glm::vec4& color)
	{
		mSprite.setColor(color);
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
