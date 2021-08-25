#ifndef DEFERRED_LIGHT_SUB_GRAPH_H
#define DEFERRED_LIGHT_SUB_GRAPH_H

#include "se/app/graphics/RenderableLight.h"
#include "ShadowRenderSubGraph.h"

namespace se::app {

	/**
	 * Class ShadowRenderSubGraph, it's the Renderer3D used for rendering the
	 * Lights. It has a Framebuffer "target" input and output where the lights
	 * will be rendererd from the Camera perspective. It also has
	 * "position", "normal", "albedo" and "material" inputs where this Textures
	 * must be attached for calculating the final light colors.
	 */
	class DeferredLightSubGraph : public graphics::Renderer3D
	{
	public:		// Nested types
		struct TexUnits
		{
			static constexpr int kPosition		= 0;
			static constexpr int kNormal		= 1;
			static constexpr int kAlbedo		= 2;
			static constexpr int kMaterial		= 3;
			static constexpr int kShadow		= 4;
		};

		class StartDLRenderNode;
		class EndDLRenderNode;

	public:		// Attributes
		/** The graph used by the DeferredLightSubGraph */
		graphics::RenderGraph mGraph;

		/** A pointer to the ShadowRenderSubGraph of the
		 * DeferredLightSubGraph */
		ShadowRenderSubGraph* mShadowRenderSubGraph = nullptr;

		/** A pointer to the Stencil RendererMesh of the
		 * DeferredLightSubGraph */
		graphics::RendererMesh* mStencilRenderer = nullptr;

		/** A pointer to the Color RendererMesh of the DeferredLightSubGraph */
		graphics::RendererMesh* mColorRenderer = nullptr;

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

		/** The bindable index of the Shadow Map used for rendering the
		 * Light Shadows in @see mGraph resources */
		std::size_t mShadowBindableIndex;

		/** The RenderQueue used for rendering the Lights Renderables */
		std::vector<RenderableLight*> mLightsRenderQueue;

	public:		// Functions
		/** Creates a new DeferredLightSubGraph
		 *
		 * @param	name the name of the RenderNode */
		DeferredLightSubGraph(const std::string& name);

		/** Class destructor */
		virtual ~DeferredLightSubGraph() = default;

		/** @return	a pointer to the ShadowRenderSubGraph of the
		 *			ShadowRenderSubGraph */
		ShadowRenderSubGraph* getShadowRenderSubGraph() const
		{ return mShadowRenderSubGraph; };

		/** @return	a pointer to the StencilRenderer of the
		 *			DeferredLightSubGraph */
		graphics::RendererMesh* getStencilRenderer() const
		{ return mStencilRenderer; };

		/** @return	a pointer to the ColorRenderer of the
		 *			DeferredLightSubGraph */
		graphics::RendererMesh* getColorRenderer() const
		{ return mColorRenderer; };

		/** @copydoc graphics::BindableRenderNode::setBindable() */
		virtual void setBindable(
			std::size_t bindableIndex, const BindableSPtr& bindable
		) override;
	protected:
		/** @copydoc Renderer::sortQueue() */
		virtual void sortQueue() override;

		/** @copydoc Renderer::render() */
		virtual void render() override;

		/** @copydoc Renderer::clearQueue() */
		virtual void clearQueue() override;

		/** @copydoc graphics::Renderer3D::submitRenderable3D() */
		virtual
		void submitRenderable3D(
			graphics::Renderable3D& renderable, graphics::Pass& pass
		) override;
	};

}

#endif		// DEFERRED_LIGHT_SUB_GRAPH_H
