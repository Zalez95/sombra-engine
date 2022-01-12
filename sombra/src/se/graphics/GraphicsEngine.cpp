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
		mRenderGraph = std::make_unique<RenderGraph>(mContext);
	}


	GraphicsEngine::~GraphicsEngine()
	{
		setRenderGraph(nullptr);
	}


	void GraphicsEngine::setRenderGraph(std::unique_ptr<RenderGraph> renderGraph)
	{
		std::scoped_lock lck(mMutex);

		mRenderGraph = std::move(renderGraph);
	}


	void GraphicsEngine::addRenderable(Renderable* renderable)
	{
		std::scoped_lock lck(mMutex);

		if (renderable) {
			mRenderables.push_back(renderable);
		}
	}


	void GraphicsEngine::removeRenderable(Renderable* renderable)
	{
		std::scoped_lock lck(mMutex);

		auto itRenderable = std::find(mRenderables.begin(), mRenderables.end(), renderable);
		if (itRenderable != mRenderables.end()) {
			if (mRenderables.size() > 1) {
				std::swap(*itRenderable, mRenderables.back());
			}
			mRenderables.pop_back();
		}
	}


	void GraphicsEngine::render()
	{
		mContext.execute([this](Context::Query& q) {
			std::scoped_lock lck(mMutex);
			for (Renderable* renderable : mRenderables) {
				renderable->submit(q);
			}
			mRenderGraph->execute(q);
		});

		mContext.update();
	}

}
