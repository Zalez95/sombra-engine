#include "se/app/graphics/RenderableLight.h"

namespace se::app {

	RenderableLight& RenderableLight::setShadows(std::size_t resolution, bool isPointLight, std::size_t numShadows)
	{
		mIsPointLight = isPointLight;
		mResolution = resolution;
		if (mIsPointLight) {
			mShadowMap = std::make_shared<graphics::Texture>(graphics::TextureTarget::CubeMap);
			for (int i = 0; i < 6; ++i) {
				mShadowMap->setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::Depth, graphics::ColorFormat::Depth, mResolution, mResolution, 0, i)
					.setWrapping(graphics::TextureWrap::Repeat, graphics::TextureWrap::Repeat, graphics::TextureWrap::Repeat)
					.setFiltering(graphics::TextureFilter::Nearest, graphics::TextureFilter::Nearest);
			}
			mViewMatrices.resize(6, glm::mat4(1.0f));
			mProjectionMatrices.resize(6, glm::mat4(1.0f));
		}
		else {
			mShadowMap = std::make_shared<graphics::Texture>(graphics::TextureTarget::Texture2DArray);
			mShadowMap->setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::Depth, graphics::ColorFormat::Depth, mResolution, mResolution, numShadows)
				.setWrapping(graphics::TextureWrap::ClampToBorder, graphics::TextureWrap::ClampToBorder, graphics::TextureWrap::ClampToBorder)
				.setFiltering(graphics::TextureFilter::Nearest, graphics::TextureFilter::Nearest)
				.setBorderColor(1.0f, 1.0f, 1.0f, 1.0f);
			mViewMatrices.resize(numShadows, glm::mat4(1.0f));
			mProjectionMatrices.resize(numShadows, glm::mat4(1.0f));
		}

		return *this;
	}


	RenderableLight& RenderableLight::disableShadows()
	{
		mIsPointLight = false;
		mResolution = 0;
		mShadowMap = nullptr;
		mViewMatrices.clear();
		mProjectionMatrices.clear();

		return *this;
	}

}
