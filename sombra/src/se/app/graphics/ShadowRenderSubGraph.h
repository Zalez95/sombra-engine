#ifndef SHADOW_RENDER_SUB_GRAPH_H
#define SHADOW_RENDER_SUB_GRAPH_H

#include "se/graphics/Renderer.h"
#include "se/graphics/RenderGraph.h"
#include "se/graphics/FBClearNode.h"
#include "se/graphics/3D/RendererMesh.h"
#include "se/graphics/3D/RendererTerrain.h"
#include "se/graphics/3D/FrustumFilter.h"
#include "IViewProjectionUpdater.h"
#include "MergeShadowsNode.h"

namespace se::app {

	/**
	 * Class ShadowUniformsUpdater, it's a IViewProjectionUpdater used for
	 * updating the view and projection matrices of the shadows
	 */
	class ShadowUniformsUpdater : public IViewProjectionUpdater
	{
	private:	// Attributes
		/** The renderers of the Passes that can be updated */
		std::vector<graphics::Renderer*> mRenderers;

	public:		// Functions
		/** Creates a new ShadowUniformsUpdater
		 *
		 * @param	viewMatUniformName the name of the View matrix uniform
		 *			variable
		* @param	projectionMatUniform Name te name of the Projection matrix
		*			uniform variable */
		ShadowUniformsUpdater(
			const char* viewMatUniformName, const char* projectionMatUniformName
		) : IViewProjectionUpdater(viewMatUniformName, projectionMatUniformName)
		{};

		/** Sets the Renderers of the Passes that can be updated */
		void setRenderers(const std::vector<graphics::Renderer*>& renderers)
		{ mRenderers = renderers; };

		/** @copydoc IViewProjectionUpdater::shouldAddUniforms() */
		virtual bool shouldAddUniforms(const PassSPtr& pass) const override;
	};


	/**
	 * Class ShadowRenderSubGraph, it's the BindableRenderNode used for
	 * rendering the Shadows. This RenderNode can't be attached to the Passes,
	 * for that you must use it conjuction with @see ShadowRendererMesh and
	 * @see ShadowRendererTerrain. It has a Framebuffer "target" input and
	 * output where the shadows can be attached and retrieved from the Camera
	 * perspective. It also has "position" and "normal" inputs where the
	 * position and normal Textures must be attached. These textures are the
	 * one rendered from the camera perspective and they're used for
	 * calculating the final shadow map.
	 */
	class ShadowRenderSubGraph : public graphics::BindableRenderNode
	{
	private:	// Nested types
		class StartShadowNode;
		class EndShadowNode;

		/** Struct ShadowRenderSubGraph, holds all the variables, resources and
		 * RenderNodes used for rendering a single Shadow */
		struct Shadow
		{
			bool active = false;

			glm::mat4 viewMatrix = glm::mat4(1.0f);
			glm::mat4 projectionMatrix = glm::mat4(1.0f);

			StartShadowNode* startNode = nullptr;
			graphics::FBClearNode* clearFB = nullptr;
			graphics::RendererMesh* rendererMesh = nullptr;
			graphics::RendererTerrain* rendererTerrain = nullptr;
			graphics::FrustumFilter* frustum = nullptr;
		};

	private:	// Attributes
		/** The graph used by the RenderNode */
		graphics::RenderGraph mGraph;

		/** The MergeShadowsNode used in @see mGraph */
		MergeShadowsNode* mMergeShadowsNode;

		/** The bindable index of the target framebuffer used for rendering the
		 * Shadows */
		std::size_t mTargetBindableIndex;

		/** The bindable index of the position texture used for rendering
		 * the Shadows */
		std::size_t mPositionTextureBindableIndex;

		/** The bindable index of the normal texture used for rendering
		 * the Shadows */
		std::size_t mNormalTextureBindableIndex;

		/** All the Shadows to render */
		std::array<Shadow, MergeShadowsNode::kMaxShadows> mShadows;

		/** The IViewProjectionUpdater used for updating the view and
		 * projection matrix uniform variables of the Passes */
		ShadowUniformsUpdater* mShadowUniformsUpdater;

	public:		// Functions
		/** Creates a new ShadowRenderSubGraph
		 *
		 * @param	name the name of the new ShadowRenderSubGraph
		 * @param	repository the Repository that holds the Resources */
		ShadowRenderSubGraph(const char* name, Repository& repository);

		/** Class destructor */
		virtual ~ShadowRenderSubGraph();

		/** @copydoc RenderNode::execute()
		 * @note	the viewport resolution will be changed after calling this
		 *			function to @see setResolution */
		virtual void execute() override;

		/** @copydoc BindableRenderNode::setBindable() */
		virtual void setBindable(
			std::size_t bindableIndex, const BindableSPtr& bindable
		) override;

		/** @return	a pointer to the ShadowUniformsUpdater used by the
		 *			ShadowRenderSubGraph to update the view and projection
		 *			matrices uniform variables */
		ShadowUniformsUpdater* getShadowUniformsUpdater() const
		{ return mShadowUniformsUpdater; };

		/** Sets the inverse view projection matrix of the camera
		 *
		 * @param	invVPMatrix the new inverse Camera view projection matrix */
		void setInvCameraViewProjectionMatrix(const glm::mat4& invVPMatrix);

		/** Sets the resolution used for rendering to the final Shadow
		 * framebuffer
		 *
		 * @param	width the width of the viewport
		 * @param	height the height of the viewport */
		void setCameraResolution(std::size_t width, std::size_t height);

		/** Adds a new Shadow to the ShadowRenderSubGraph
		 *
		 * @param	resolution the resolution used for the shadow maps
		 * @param	viewMatrix the view matrix used for rendering the shadow
		 * @param	projectionMatrix the projection matrix used for rendering
		 *			the shadow
		 * @return	the index of the shadow, if it can't be added more textures
		 *			this index will be larger than
		 *			@see MergeShadowsNode::kMaxShadows */
		std::size_t addShadow(
			std::size_t resolution,
			const glm::mat4& viewMatrix = glm::mat4(1.0f),
			const glm::mat4& projectionMatrix = glm::mat4(1.0f)
		);

		/** Changes the view projection matrices of the shadow map of the given
		 * shadow
		 *
		 * @param	shadowIndex the index of the shadow to update
		 * @param	viewMatrix the view matrix used for rendering the shadow
		 * @param	projectionMatrix the projection matrix used for rendering
		 *			the shadow */
		void setShadowVPMatrix(
			std::size_t shadowIndex,
			const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix
		);

		/** Changes the resolution of the shadow map of the given shadow
		 *
		 * @param	shadowIndex the index of the shadow to update
		 * @param	resolution the new resolution of the shadow map */
		void setShadowResolution(std::size_t shadowIndex, std::size_t resolution);

		/** Removes the given Shadow from the ShadowRenderSubGraph
		 *
		 * @param	shadowIndex the index of the shadow to update */
		void removeShadow(std::size_t shadowIndex);

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

		/** @copydoc graphics::RenderNode::execute() */
		virtual void execute() override {};
	protected:
		/** @copydoc graphics::Renderer3D::sortQueue() */
		virtual void sortQueue() override {};

		/** @copydoc graphics::Renderer3D::render() */
		virtual void render() override {};

		/** @copydoc graphics::Renderer3D::clearQueue() */
		virtual void clearQueue() override {};
	};


	/**
	 * Class ShadowRendererMesh, it's a ShadowProxyRenderer used for
	 * submitting RenderableMeshes
	 */
	class ShadowRendererMesh : public ShadowProxyRenderer
	{
	public:		// Functions
		/** Creates a new ShadowRendererMesh
		 *
		 * @param	name the name of the new ShadowRendererMesh
		 * @param	subGraph the ShadowRenderSubGraph that will be used for
		 *			rendering the Shadows */
		ShadowRendererMesh(
			const std::string& name, ShadowRenderSubGraph& subGraph
		) : ShadowProxyRenderer(name, subGraph) {};

		/** @copydoc graphics::Renderer3D::submit() */
		virtual void submit(
			graphics::Renderable& renderable, graphics::Pass& pass
		) { mShadowRenderSubGraph.submitMesh(renderable, pass); };
	};


	/**
	 * Class ShadowRendererTerrain, it's a ShadowProxyRenderer used for
	 * submitting RenderableTerrains
	 */
	class ShadowRendererTerrain : public ShadowProxyRenderer
	{
	public:		// Functions
		/** Creates a new ShadowRendererTerrain
		 *
		 * @param	name the name of the new ShadowRendererTerrain
		 * @param	subGraph the ShadowRenderSubGraph that will be used for
		 *			rendering the Shadows */
		ShadowRendererTerrain(
			const std::string& name, ShadowRenderSubGraph& subGraph
		) : ShadowProxyRenderer(name, subGraph) {};

		/** @copydoc graphics::Renderer3D::submit() */
		virtual void submit(
			graphics::Renderable& renderable, graphics::Pass& pass
		) { mShadowRenderSubGraph.submitTerrain(renderable, pass); };
	};

}

#endif		// SHADOW_RENDER_SUB_GRAPH_H
