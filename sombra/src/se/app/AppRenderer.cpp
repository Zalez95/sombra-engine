#include "se/utils/Log.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/app/graphics/AppRenderGraph.h"
#include "se/app/AppRenderer.h"
#include "se/app/Application.h"

namespace se::app {

	AppRenderer::AppRenderer(Application& application, std::size_t width, std::size_t height) :
		ISystem(application.getEntityDatabase()), mApplication(application)
	{
		mApplication.getEventManager().subscribe(this, Topic::WindowResize);
		mApplication.getEventManager().subscribe(this, Topic::RendererResolution);

		SOMBRA_INFO_LOG << graphics::GraphicsOperations::getGraphicsInfo();
		auto graph = std::make_unique<AppRenderGraph>(application.getRepository(), width, height);
		mApplication.getExternalTools().graphicsEngine->setRenderGraph(std::move(graph));

		graphics::GraphicsOperations::setViewport(0, 0, width, height);
	}


	AppRenderer::~AppRenderer()
	{
		mApplication.getExternalTools().graphicsEngine->getRenderGraph().clearNodes();
		mApplication.getEventManager().unsubscribe(this, Topic::RendererResolution);
		mApplication.getEventManager().unsubscribe(this, Topic::WindowResize);
	}


	bool AppRenderer::notify(const IEvent& event)
	{
		return tryCall(&AppRenderer::onWindowResizeEvent, event)
			|| tryCall(&AppRenderer::onRendererResolutionEvent, event);
	}


	void AppRenderer::render()
	{
		SOMBRA_DEBUG_LOG << "Render start";
		mApplication.getExternalTools().graphicsEngine->render();
		SOMBRA_DEBUG_LOG << "Render end";
	}

// Private functions
	void AppRenderer::onWindowResizeEvent(const WindowResizeEvent& event)
	{
		SOMBRA_INFO_LOG << event;

		auto width = static_cast<std::size_t>(event.getWidth());
		auto height = static_cast<std::size_t>(event.getHeight());
		graphics::GraphicsOperations::setViewport(0, 0, width, height);
	}


	void AppRenderer::onRendererResolutionEvent(const RendererResolutionEvent& event)
	{
		SOMBRA_INFO_LOG << event;

		auto graphicsEngine = mApplication.getExternalTools().graphicsEngine;
		if (auto graph = dynamic_cast<AppRenderGraph*>(&graphicsEngine->getRenderGraph())) {
			auto width = static_cast<std::size_t>(event.getWidth());
			auto height = static_cast<std::size_t>(event.getHeight());
			graph->setResolution(width, height);
		}
	}

}
