#ifndef DEFERRED_LIGHT_SUB_GRAPH_H
#define DEFERRED_LIGHT_SUB_GRAPH_H

#include "se/graphics/3D/RendererMesh.h"
#include "se/graphics/RenderGraph.h"

namespace se::app {

	/**
	 * Class ShadowRenderSubGraph, it's the Renderer used for rendering the
	 * Lights. This RenderNode must not be attached to the Passes, for that
	 * you must use it conjuction with @see LightStencilRenderer and
	 * @see LightColorRenderer. It has a Framebuffer "target" input and
	 * output where the lights will be rendererd from the Camera perspective.
	 * It also has "position", "normal", "albedo" and "material" inputs where
	 * this Textures must be attached for calculating the final light colors.
	 */
	class DeferredLightSubGraph : public graphics::Renderer
	{
	public:		// Nested types
		struct TexUnits
		{
			static constexpr int kPosition		= 0;
			static constexpr int kNormal		= 1;
			static constexpr int kAlbedo		= 2;
			static constexpr int kMaterial		= 3;
		};
	private:
		struct RenderQueueData
		{
			graphics::Renderable* renderable = nullptr;
			graphics::Pass* pass = nullptr;
			bool stencil = false;
		};

	private:	// Attributes
		/** The graph used by the DeferredLightSubGraph */
		graphics::RenderGraph mGraph;

		/** A pointer to the Stencil RendererMesh of the sub graph */
		graphics::RendererMesh* mStencilRenderer;

		/** A pointer to the Color RendererMesh of the sub graph */
		graphics::RendererMesh* mColorRenderer;

		/** The bindable index of the target framebuffer used for rendering the
		 * Lights */
		std::size_t mTargetBindableIndex;

		/** The bindable index of the position texture used for rendering
		 * the Lights */
		std::size_t mPositionTextureBindableIndex;

		/** The bindable index of the normal texture used for rendering
		 * the Lights */
		std::size_t mNormalTextureBindableIndex;

		/** The bindable index of the albedo texture used for rendering
		 * the Lights */
		std::size_t mAlbedoTextureBindableIndex;

		/** The bindable index of the material texture used for rendering
		 * the Lights */
		std::size_t mMaterialTextureBindableIndex;

		/** The RenderQueue used for rendering the Lights Renderables */
		std::vector<RenderQueueData> mLightsRenderQueue;

	public:
		/** Creates a new DeferredLightSubGraph
		 *
		 * @param	name the name of the RenderNode */
		DeferredLightSubGraph(const std::string& name);

		/** Class destructor */
		virtual ~DeferredLightSubGraph() = default;

		/** @copydoc graphics::BindableRenderNode::setBindable() */
		virtual void setBindable(
			std::size_t bindableIndex, const BindableSPtr& bindable
		) override;

		/** @copydoc graphics::Renderer::submit() */
		virtual void submit(
			graphics::Renderable& renderable, graphics::Pass& pass
		) override;

		/** Submits a Renderable and it's pass to the DeferredLightSubGraph's
		 * StencilRenderer
		 *
		 * @param	renderable the Light RenderableMesh to draw
		 * @param	pass the Pass used for rendering the Renderable */
		void submitStencil(
			graphics::Renderable& renderable, graphics::Pass& pass
		);

		/** Submits a Renderable and it's pass to the DeferredLightSubGraph's
		 * ColorRenderer
		 *
		 * @param	renderable the Light RenderableMesh to draw
		 * @param	pass the Pass used for rendering the Renderable */
		void submitColor(
			graphics::Renderable& renderable, graphics::Pass& pass
		);
	protected:
		/** @copydoc Renderer::sortQueue() */
		virtual void sortQueue() override;

		/** @copydoc Renderer::render() */
		virtual void render() override;

		/** @copydoc Renderer::clearQueue() */
		virtual void clearQueue() override;
	};


	/**
	 * Class DeferredLightProxyRenderer. It's a Renderer that doesn't render,
	 * it only acts as link between the Light Passes and the
	 * DeferredLightSubGraph. It will be attached to the LightPasses, and when
	 * the Renderables are submitted to this Renderer, it will pass them to the
	 * DeferredLightSubGraph.
	 */
	class DeferredLightProxyRenderer : public graphics::Renderer
	{
	protected:	// Attributes
		/** The DeferredLightSubGraph that will be used for rendering the
		 * Lights */
		DeferredLightSubGraph& mDeferredLightSubGraph;

	public:		// Functions
		/** Creates a new DeferredLightProxyRenderer
		 *
		 * @param	name the name of the new DeferredLightProxyRenderer
		 * @param	subGraph the ShadowRenderSubGraph that will be used
		 *			for rendering the Lights */
		DeferredLightProxyRenderer(
			const std::string& name, DeferredLightSubGraph& subGraph
		);

		/** Class destructor */
		~DeferredLightProxyRenderer() = default;

		/** @copydoc graphics::RenderNode::execute() */
		virtual void execute() override {};
	protected:
		/** @copydoc graphics::Renderer::sortQueue() */
		virtual void sortQueue() override {};

		/** @copydoc graphics::Renderer::render() */
		virtual void render() override {};

		/** @copydoc graphics::Renderer::clearQueue() */
		virtual void clearQueue() override {};
	};


	/**
	 * Class LightStencilRenderer, it's a DeferredLightProxyRenderer used for
	 * submitting Lights Renderables to the StencilRenderer of the
	 * DeferredLightSubGraph
	 */
	class LightStencilRenderer : public DeferredLightProxyRenderer
	{
	public:		// Functions
		/** Creates a new LightStencilRenderer
		 *
		 * @param	name the name of the new LightStencilRenderer
		 * @param	subGraph the DeferredLightSubGraph that will be used for
		 *			rendering the light Renderables */
		LightStencilRenderer(
			const std::string& name, DeferredLightSubGraph& subGraph
		) : DeferredLightProxyRenderer(name, subGraph) {};

		/** @copydoc graphics::Renderer::submit() */
		virtual void submit(
			graphics::Renderable& renderable, graphics::Pass& pass
		) { mDeferredLightSubGraph.submitStencil(renderable, pass); };
	};


	/**
	 * Class LightColorRenderer, it's a DeferredLightProxyRenderer used for
	 * submitting Lights Renderables to the ColorRenderer of the
	 * DeferredLightSubGraph
	 */
	class LightColorRenderer : public DeferredLightProxyRenderer
	{
	public:		// Functions
		/** Creates a new LightColorRenderer
		 *
		 * @param	name the name of the new LightColorRenderer
		 * @param	subGraph the DeferredLightSubGraph that will be used for
		 *			rendering the light Renderables */
		LightColorRenderer(
			const std::string& name, DeferredLightSubGraph& subGraph
		) : DeferredLightProxyRenderer(name, subGraph) {};

		/** @copydoc graphics::Renderer::submit() */
		virtual void submit(
			graphics::Renderable& renderable, graphics::Pass& pass
		) { mDeferredLightSubGraph.submitColor(renderable, pass); };
	};

}

#endif		// DEFERRED_LIGHT_SUB_GRAPH_H
