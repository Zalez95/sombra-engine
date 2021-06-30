#include "se/app/events/ContainerEvent.h"
#include "se/app/LightComponent.h"

namespace se::app {

	LightComponent::LightComponent(const LightComponent& other) : mSource(other.mSource)
	{
		if (other.mShadowData) {
			mShadowData = std::make_unique<ShadowData>(*other.mShadowData);
		}
	}


	LightComponent& LightComponent::operator=(const LightComponent& other)
	{
		mEventManager = nullptr;
		mEntity = kNullEntity;
		mSource = other.mSource;
		if (other.mShadowData) {
			mShadowData = std::make_unique<ShadowData>(*other.mShadowData);
		}
		mShadowIndices = 0;

		return *this;
	}


	void LightComponent::setSource(const Repository::ResourceRef<LightSource>& source)
	{
		mSource = std::move(source);
		if (mShadowData) {
			setShadowData(nullptr);
		}
	}


	void LightComponent::setShadowData(std::unique_ptr<ShadowData> data)
	{
		if (mSource) {
			mShadowData = std::move(data);
			if (mEventManager) {
				mEventManager->publish(new se::app::ContainerEvent<se::app::Topic::Shadow, se::app::Entity>(mEntity));
			}
		}
	}

}
