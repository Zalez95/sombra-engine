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
		auto graph = std::make_unique<AppRenderGraph>(mApplication.getExternalTools().graphicsEngine->getContext(), width, height);
		mApplication.getExternalTools().graphicsEngine->setRenderGraph(std::move(graph));

		mApplication.getExternalTools().graphicsEngine->getContext().execute([=](graphics::Context::Query&) {
			graphics::GraphicsOperations::setViewport(0, 0, width, height);
		});
	}


	AppRenderer::~AppRenderer()
	{
		mApplication.getExternalTools().graphicsEngine->editRenderGraph([](graphics::RenderGraph& graph) {
			graph.clearNodes();
		});

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
		mApplication.getExternalTools().graphicsEngine->getContext().execute([=](graphics::Context::Query&) {
			graphics::GraphicsOperations::setViewport(0, 0, width, height);
		});
	}


	void AppRenderer::onRendererResolutionEvent(const RendererResolutionEvent& event)
	{
		SOMBRA_INFO_LOG << event;

		mApplication.getExternalTools().graphicsEngine->editRenderGraph([&](graphics::RenderGraph& graph) {
			auto appGraph = dynamic_cast<AppRenderGraph*>(&graph);
			if (appGraph) {
				auto width = static_cast<std::size_t>(event.getWidth());
				auto height = static_cast<std::size_t>(event.getHeight());
				appGraph->setResolution(width, height);
			}
		});
	}

}
