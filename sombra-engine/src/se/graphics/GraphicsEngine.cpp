#include <stdexcept>
#include <algorithm>
#include "se/graphics/core/GraphicsOperations.h"
#include "se/graphics/GraphicsEngine.h"

namespace se::graphics {

	GraphicsEngine::GraphicsEngine()
	{
		if (!GraphicsOperations::init()) {
			throw std::runtime_error("Failed to initialize the Graphics API");
		}

		// Create the Renderers
		mRenderGraph = std::make_unique<RenderGraph>();
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

		for (Renderable* renderable : mRenderables) {
			renderable->submit();
		}

		mRenderGraph->execute();
	}

}
