#ifndef APP_RENDERER_H
#define APP_RENDERER_H

#include "events/ResizeEvent.h"
#include "events/EventManager.h"
#include "ECS.h"

namespace se::app {

	class Application;


	/**
	 * Class AppRenderer, It's a System used for creating the RenderGraph and
	 * rendering the Entities
	 * @note	the Passes can use either the "forwardRendererMesh", or the
	 *			"gBufferRendererTerrain", "gBufferRendererMesh" or
	 *			"gBufferRendererTerrain" of the RenderGraph for submitting the
	 *			Renderable3Ds. The gBufferRenderers are used for rendering
	 *			geometry in a deferred pipeline, the passes that uses it
	 *			must output position, normal, albedo, material and emissive
	 *			textures in that order, later this textures will be used for
	 *			rendering in a PBR pipeline. The "forwardRendererMesh" is
	 *			reserved for special cases that can't be rendered this way.
	 *			For the Renderable2Ds, there is a "renderer2D" for submitting
	 *			them */
	class AppRenderer : public ISystem, public IEventListener
	{
	private:	// Attributes
		/** The Application that holds the GraphicsEngine used for rendering
		 * the Entities */
		Application& mApplication;

	public:		// Functions
		/** Creates a new AppRenderer
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System
		 * @param	width the initial width of the FrameBuffer where the
		 *			Entities are going to be rendered
		 * @param	height the initial height of the FrameBuffer where the
		 *			Entities are going to be rendered */
		AppRenderer(
			Application& application, std::size_t width, std::size_t height
		);

		/** Class destructor */
		~AppRenderer();

		/** @copydoc IEventListener::notify(const IEvent&) */
		virtual bool notify(const IEvent& event) override;

		/** Renders the graphics data of the Entities
		 *
		 * @note	this function must be called from the thread with the
		 *			Graphics API context (probably thread 0) */
		void render();
	private:
		/** Handles the given WindowResizeEvent by notifying the GraphicsEngine
		 * of the window resize
		 *
		 * @param	event the WindowResizeEvent to handle */
		void onWindowResizeEvent(const WindowResizeEvent& event);

		/** Handles the given RendererResolutionEvent by notifying the
		 * GraphicsEngine of a change in the renderer resolution
		 *
		 * @param	event the RendererResolutionEvent to handle */
		void onRendererResolutionEvent(const RendererResolutionEvent& event);
	};

}

#endif		// APP_RENDERER_H
