#ifndef SHADOW_RENDER_SUB_GRAPH_H
#define SHADOW_RENDER_SUB_GRAPH_H

#include "se/graphics/RenderGraph.h"
#include "se/graphics/3D/RendererMesh.h"
#include "se/graphics/3D/RendererTerrain.h"
#include "se/graphics/3D/FrustumFilter.h"
#include "se/app/graphics/RenderableLight.h"
#include "IViewProjectionUpdater.h"

namespace se::app {

	/**
	 * Class ShadowUniformsUpdater, it's a IViewProjectionUpdater used for
	 * updating the view and projection matrices of the shadows
	 */
	class ShadowUniformsUpdater : public IViewProjectionUpdater
	{
	private:	// Attributes
		/** The ShadowRenderSubGraph used by the Proxies */
		const graphics::RenderNode* mRenderNode;

	public:		// Functions
		/** Creates a new ShadowUniformsUpdater
		 *
		 * @param	context the graphics Context used for creating the uniforms
		 * @param	viewMatUniformName the name of the View matrix uniform
		 *			variable
		* @param	projectionMatUniform Name te name of the Projection
		*			matrix uniform variable
		* @param	renderNode the RenderNode of the Passes tha can be
		*			updated */
		ShadowUniformsUpdater(
			graphics::Context& context,
			const char* viewMatUniformName,
			const char* projectionMatUniformName,
			const graphics::RenderNode* renderNode
		) : IViewProjectionUpdater(
				context, viewMatUniformName, projectionMatUniformName
			), mRenderNode(renderNode) {};

		/** @copydoc IViewProjectionUpdater::shouldAddUniforms() */
		virtual bool shouldAddUniforms(const PassSPtr& pass) const override;
	};


	/**
	 * Class ShadowRenderSubGraph, it's the BindableRenderNode used for
	 * rendering the Shadows. This RenderNode can't be attached to the Passes,
	 * for that you must use it conjuction with @see ShadowMeshProxyRenderer and
	 * @see ShadowTerrainProxyRenderer. It only has a "shadow" Texture output.
	 * This subgraph internally has the following structure:
	 *      [ resources ]
	 *            |shadowTarget
	 *            |
	 *            |target
	 *        [fbClear]
	 *            |target
	 *            |
	 *            |target
	 * [shadowTerrainRenderer]
	 *            |target
	 *            |
	 *            |target
	 *   [shadowMeshRenderer]
	 */
	class ShadowRenderSubGraph : public graphics::BindableRenderNode
	{
	private:	// Nested types
		struct RenderQueueData
		{
			graphics::Renderable* renderable = nullptr;
			graphics::Pass* pass = nullptr;
		};

	private:	// Attributes
		/** The RenderGraph used for drawing the Shadows */
		graphics::RenderGraph mGraph;

		/** A pointer to the resources node of @see mGraph */
		graphics::BindableRenderNode* mResources = nullptr;

		/** A pointer to the RendererTerrain of @see mGraph */
		graphics::RendererTerrain* mShadowTerrainRenderer = nullptr;

		/** A pointer to the RendererMesh of @see mGraph */
		graphics::RendererMesh* mShadowMeshRenderer = nullptr;

		/** The frustum used for filtering Renderables by the Renderers */
		graphics::FrustumFilter* mRenderersFrustum = nullptr;

		/** The index of the ShadowMap Bindable resource in @see mResources */
		std::size_t mShadowTargetBindableIndex;

		/** The index of the ShadowMap Bindable output */
		std::size_t mShadowBindableIndex;

		/** The Render queue used for drawing the RenderableTerrains to the
		 * Shadows */
		std::vector<RenderQueueData> mTerrainRenderQueue;

		/** The Render queue used for drawing the RenderableMeshes to the
		 * Shadows */
		std::vector<RenderQueueData> mMeshRenderQueue;

		/** The IViewProjectionUpdater used for updating the view and
		 * projection matrix uniform variables of the Passes */
		ShadowUniformsUpdater mShadowUniformsUpdater;

		/** The width and height of the viewport */
		std::size_t mWidth, mHeight;

		/** A pointer to the RenderableLight used for rendering the Shadows */
		const RenderableLight* mRenderableLight = nullptr;

	public:		// Functions
		/** Creates a new ShadowRenderSubGraph
		 *
		 * @param	name the name of the new ShadowRenderSubGraph
		 * @param	context the graphics Context used for creating the
		 *			uniforms */
		ShadowRenderSubGraph(
			const std::string& name, graphics::Context& context
		);

		/** Sets the resolution of the viewport after the Shadows have been
		 * drawn
		 *
		 * @param	width the new width of the viewport
		 * @param	height the new height of the viewport */
		void setResolution(std::size_t width, std::size_t height)
		{ mWidth = width; mHeight = height; };

		/** @return	a pointer to the ShadowUniformsUpdater of the
		 *			ShadowRenderSubGraph */
		ShadowUniformsUpdater& getShadowUniformsUpdater()
		{ return mShadowUniformsUpdater; };

		/** Prepares the ShadowRenderSubGraph for a new render
		 *
		 * @param	renderableLight the RenderableLight used for rendering the
		 *			Shadows
		 * @note	if the renderableLight doesn't casts shadows, the
		 *			ShadowRenderSubGraph won't do anything on @see execute */
		void startRender(const RenderableLight& renderableLight);

		/** @copydoc graphics::RenderNode::execute(graphics::Context::Query&) */
		virtual void execute(graphics::Context::Query& q) override;

		/** Clears the ShadowRenderSubGraph after a render */
		void endRender();

		/** Clears the Queues of the ShadowRenderSubGraph after all the Renders
		 * of a frame have been done */
		void clearQueues();

		/** Submits a Renderable and it's pass to the ShadowRenderSubGraph's
		 * RendererTerrains
		 *
		 * @param	renderable the RenderableMesh to draw
		 * @param	pass the Pass used for rendering the Renderable */
		void submitTerrain(
			graphics::Renderable& renderable, graphics::Pass& pass
		);

		/** Submits a Renderable and it's pass to the ShadowRenderSubGraph's
		 * RendererMeshes
		 *
		 * @param	renderable the RenderableTerrain to draw
		 * @param	pass the Pass used for rendering the Renderable */
		void submitMesh(
			graphics::Renderable& renderable, graphics::Pass& pass
		);
	};


	/**
	 * Class ShadowProxyRenderer. It's a Renderer that doesn't render, it only
	 * acts as link between the Shadow Passes and the ShadowRenderSubGraph.
	 * It will be attached to the ShadowPasses, and when the Renderables are
	 * submitted to this Renderer, it will pass them to the
	 * ShadowRenderSubGraph.
	 */
	class ShadowProxyRenderer : public graphics::Renderer
	{
	protected:	// Attributes
		/** The ShadowRenderSubGraph that will be used for rendering the
		 * Shadows */
		ShadowRenderSubGraph& mShadowRenderSubGraph;

	public:		// Functions
		/** Creates a new ShadowProxyRenderer
		 *
		 * @param	name the name of the new ShadowProxyRenderer
		 * @param	subGraph the ShadowRenderSubGraph that will be used for
		 *			rendering the Shadows */
		ShadowProxyRenderer(
			const std::string& name, ShadowRenderSubGraph& subGraph
		);

		/** Class destructor */
		~ShadowProxyRenderer() = default;

		/** @return the ShadowRenderSubGraph of the ShadowProxyRenderer */
		const ShadowRenderSubGraph& getShadowRenderSubGraph()
		{ return mShadowRenderSubGraph; };

		/** @copydoc graphics::RenderNode::execute(graphics::Context::Query&) */
		virtual void execute(graphics::Context::Query&) override {};
	protected:
		/** @copydoc graphics::Renderer::sortQueue() */
		virtual void sortQueue() override {};

		/** @copydoc graphics::Renderer::render(graphics::Context::Query&) */
		virtual void render(graphics::Context::Query&) override {};

		/** @copydoc graphics::Renderer::clearQueue() */
		virtual void clearQueue() override {};
	};


	/**
	 * Class ShadowTerrainProxyRenderer, it's a ShadowProxyRenderer used for
	 * submitting RenderableTerrains to the ShadowTerrainProxyRenderer of the
	 * ShadowProxyRenderer
	 */
	class ShadowTerrainProxyRenderer : public ShadowProxyRenderer
	{
	public:		// Functions
		/** Creates a new ShadowTerrainProxyRenderer
		 *
		 * @param	name the name of the new ShadowTerrainProxyRenderer
		 * @param	subGraph the ShadowRenderSubGraph that will be used for
		 *			rendering the Shadows */
		ShadowTerrainProxyRenderer(
			const std::string& name, ShadowRenderSubGraph& subGraph
		) : ShadowProxyRenderer(name, subGraph) {};

		/** @copydoc graphics::Renderer::submit() */
		virtual void submit(
			graphics::Renderable& renderable, graphics::Pass& pass
		) override { mShadowRenderSubGraph.submitTerrain(renderable, pass); };
	};


	/**
	 * Class ShadowMeshProxyRenderer, it's a ShadowProxyRenderer used for
	 * submitting RenderableTerrains to the ShadowMeshProxyRenderer of the
	 * ShadowProxyRenderer
	 */
	class ShadowMeshProxyRenderer : public ShadowProxyRenderer
	{
	public:		// Functions
		/** Creates a new ShadowMeshProxyRenderer
		 *
		 * @param	name the name of the new ShadowMeshProxyRenderer
		 * @param	subGraph the ShadowRenderSubGraph that will be used for
		 *			rendering the Shadows */
		ShadowMeshProxyRenderer(
			const std::string& name, ShadowRenderSubGraph& subGraph
		) : ShadowProxyRenderer(name, subGraph) {};

		/** @copydoc graphics::Renderer::submit() */
		virtual void submit(
			graphics::Renderable& renderable, graphics::Pass& pass
		) override { mShadowRenderSubGraph.submitMesh(renderable, pass); };
	};

}

#endif		// SHADOW_RENDER_SUB_GRAPH_H
