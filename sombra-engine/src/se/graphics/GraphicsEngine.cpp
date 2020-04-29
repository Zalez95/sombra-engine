#include <stdexcept>
#include <algorithm>
#include "se/graphics/core/GraphicsOperations.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/graphics/Renderable.h"

namespace se::graphics {

	GraphicsEngine::GraphicsEngine(const GraphicsData& config)
	{
		if (!GraphicsOperations::init()) {
			throw std::runtime_error("Failed to initialize the Graphics API");
		}

		// Set the initial viewport size
		setViewportSize(config.viewportSize);

		// Create the Renderers
		mRenderer2D = std::make_unique<Renderer2D>();
		mRenderer3D = std::make_unique<Renderer3D>();
	}


	std::string GraphicsEngine::getGraphicsInfo() const
	{
		return GraphicsOperations::getGraphicsInfo();
	}


	void GraphicsEngine::setViewportSize(const glm::uvec2& viewportSize)
	{
		std::unique_lock lck(mMutex);

		mViewportSize = viewportSize;
		GraphicsOperations::setViewport(0, 0, mViewportSize.x, mViewportSize.y);
	}


	void GraphicsEngine::addRenderable(Renderable* renderable)
	{
		std::unique_lock lck(mMutex);

		if (renderable) {
			mRenderables.push_back(renderable);
		}
	}


	void GraphicsEngine::removeRenderable(Renderable* renderable)
	{
		std::unique_lock lck(mMutex);

		mRenderables.erase(
			std::remove(mRenderables.begin(), mRenderables.end(), renderable),
			mRenderables.end()
		);
	}


	void GraphicsEngine::render()
	{
		std::unique_lock lck(mMutex);

		GraphicsOperations::clear(false, true);

		for (Renderable* renderable : mRenderables) {
			renderable->submit();
		}

		mRenderer3D->render();
		mRenderer2D->render();
	}

}
