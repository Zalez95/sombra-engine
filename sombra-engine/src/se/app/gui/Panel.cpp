#include "se/utils/Repository.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/app/gui/Panel.h"
#include "se/app/gui/GUIManager.h"
#include "se/app/Application.h"

namespace se::app {

	Panel::Panel(GUIManager* guiManager) :
		mGUIManager(guiManager), mSprite(mPosition, mSize)
	{
		mSprite.setZIndex(mZIndex);
		auto& application = mGUIManager->getApplication();
		auto technique2D = application.getRepository().find<std::string, graphics::Technique>("technique2D");
		if (technique2D) {
			mSprite.addTechnique(technique2D);
		}

		mIsVisible = false;
		setVisibility(true);
	}


	Panel::Panel(const Panel& other) :
		ComposedComponent(other),
		mGUIManager(other.mGUIManager),
		mSprite(other.mSprite)
	{
		mIsVisible = false;
		setVisibility(other.mIsVisible);
	}


	Panel::~Panel()
	{
		setVisibility(false);
	}


	Panel& Panel::operator=(const Panel& other)
	{
		ComposedComponent::operator=(other);
		mGUIManager = other.mGUIManager;
		mSprite = other.mSprite;

		mIsVisible = false;
		setVisibility(other.mIsVisible);

		return *this;
	}


	void Panel::setPosition(const glm::vec2& position)
	{
		ComposedComponent::setPosition(position);
		mSprite.setPosition(mPosition);
	}


	void Panel::setSize(const glm::vec2& size)
	{
		ComposedComponent::setSize(size);
		mSprite.setSize(mSize);
	}


	void Panel::setZIndex(unsigned char zIndex)
	{
		ComposedComponent::setZIndex(zIndex);
		mSprite.setZIndex(mZIndex);
	}


	void Panel::setVisibility(bool isVisible)
	{
		bool wasVisible = mIsVisible;
		ComposedComponent::setVisibility(isVisible);

		auto& application = mGUIManager->getApplication();
		if (wasVisible && !mIsVisible) {
			application.getExternalTools().graphicsEngine->removeRenderable(&mSprite);
		}
		else if (!wasVisible && mIsVisible) {
			application.getExternalTools().graphicsEngine->addRenderable(&mSprite);
		}
	}


	void Panel::setColor(const glm::vec4& color)
	{
		mSprite.setColor(color);
	}

}
