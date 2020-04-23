#include <stdexcept>
#include <algorithm>
#include "se/graphics/GraphicsEngine.h"
#include "se/graphics/core/Graphics.h"

namespace se::graphics {

	GraphicsEngine::GraphicsEngine(const GraphicsData& config)
	{
		if (!Graphics::init()) {
			throw std::runtime_error("Failed to initialize the Graphics API");
		}

		Graphics::setCulling(true);
		Graphics::setDepthTest(true);

		// Create the Repositories
		mTextureRepo = std::make_unique<Texture::Repository>(config.maxTextures);
		mFontRepo = std::make_unique<Font::Repository>(config.maxFonts);
		mMaterialRepo = std::make_unique<Material::Repository>(config.maxMaterials);
		mSplatmapMaterialRepo = std::make_unique<SplatmapMaterial::Repository>(config.maxSplatmapMaterials);

		// Set the initial viewport size
		setViewportSize(config.viewportSize);
	}


	std::string GraphicsEngine::getGLInfo() const
	{
		return Graphics::getGraphicsInfo();
	}


	void GraphicsEngine::setViewportSize(const glm::uvec2& viewportSize)
	{
		mViewportSize = viewportSize;
		Graphics::setViewport(0, 0, mViewportSize.x, mViewportSize.y);
	}


	void GraphicsEngine::addLayer(ILayer* layer)
	{
		if (layer) {
			mLayers.push_back(layer);
		}
	}


	void GraphicsEngine::removeLayer(ILayer* layer)
	{
		mLayers.erase(
			std::remove(mLayers.begin(), mLayers.end(), layer),
			mLayers.end()
		);
	}


	void GraphicsEngine::render()
	{
		Graphics::clear(false, true);

		for (ILayer* layer : mLayers) {
			layer->render();
		}
	}

}
