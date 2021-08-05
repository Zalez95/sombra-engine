#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/graphics/FBClearNode.h"
#include "ShadowRenderSubGraph.h"

namespace se::app {

	bool ShadowUniformsUpdater::shouldAddUniforms(const PassSPtr& pass) const
	{
		auto proxy = dynamic_cast<ShadowProxyRenderer*>(&pass->getRenderer());
		return proxy && (&proxy->getShadowRenderSubGraph() == mRenderNode);
	}


	ShadowRenderSubGraph::ShadowRenderSubGraph(const std::string& name) :
		graphics::BindableRenderNode(name), mShadowUniformsUpdater("uViewMatrix", "uProjectionMatrix", this)
	{
		mResources = dynamic_cast<BindableRenderNode*>(mGraph.getNode("resources"));

		auto shadowTarget = std::make_unique<graphics::FrameBuffer>();
		shadowTarget->setColorBuffer(false);
		mShadowTargetBindableIndex = mResources->addBindable( std::move(shadowTarget) );
		mResources->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::FrameBuffer>>("shadowTarget", mResources, mShadowTargetBindableIndex) );

		auto fbClear = std::make_unique<graphics::FBClearNode>("fbClear", graphics::FrameBufferMask::Mask().set(graphics::FrameBufferMask::kDepth));

		auto frustum = std::make_shared<graphics::FrustumFilter>();
		mRenderersFrustum = frustum.get();

		auto shadowTerrainRenderer = std::make_unique<graphics::RendererTerrain>("shadowTerrainRenderer");
		shadowTerrainRenderer->addBindable(std::make_shared<graphics::SetOperation>(graphics::Operation::DepthTest, true));
		shadowTerrainRenderer->addBindable(std::make_shared<graphics::SetOperation>(graphics::Operation::Culling, true));
		shadowTerrainRenderer->addFilter(frustum);
		mShadowTerrainRenderer = shadowTerrainRenderer.get();

		auto shadowMeshRenderer = std::make_unique<graphics::RendererMesh>("shadowMeshRenderer");
		shadowMeshRenderer->addBindable(std::make_shared<graphics::SetOperation>(graphics::Operation::DepthTest, true));
		shadowMeshRenderer->addBindable(std::make_shared<graphics::SetOperation>(graphics::Operation::Culling, true));
		shadowMeshRenderer->addFilter(frustum);
		mShadowMeshRenderer = shadowMeshRenderer.get();

		if (fbClear->findInput("input")->connect( mResources->findOutput("shadowTarget") )
			&& shadowTerrainRenderer->findInput("target")->connect( fbClear->findOutput("output") )
			&& shadowMeshRenderer->findInput("target")->connect( shadowTerrainRenderer->findOutput("target") )
			&& mGraph.addNode( std::move(fbClear) )
			&& mGraph.addNode( std::move(shadowTerrainRenderer) )
			&& mGraph.addNode( std::move(shadowMeshRenderer) )
		) {
			mGraph.prepareGraph();

			mShadowBindableIndex = addBindable(nullptr, false);
			addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("shadow", this, mShadowBindableIndex) );
		}
	}


	void ShadowRenderSubGraph::startRender(const RenderableLight& renderableLight)
	{
		if (renderableLight.castsShadows()) {
			mRenderableLight = &renderableLight;
			setBindable(mShadowBindableIndex, renderableLight.getShadowMap());
		}
	}


	void ShadowRenderSubGraph::execute()
	{
		if (!mRenderableLight) { return; }

		// Set the viewport size to the RenderableLight ShadowMap resolution
		graphics::GraphicsOperations::setViewport(0, 0, mRenderableLight->getResolution(), mRenderableLight->getResolution());
		graphics::GraphicsOperations::setCullingMode(graphics::FaceMode::Front);	// Render the back faces

		// Render to the ShadowMap textures from the RenderableLight perspectives
		auto shadowTarget = std::dynamic_pointer_cast<graphics::FrameBuffer>( mResources->getBindable(mShadowTargetBindableIndex) );
		glm::mat4 viewMatrix(1.0f), projectionMatrix(1.0f), viewProjectionMatrix(1.0f);

		for (std::size_t i = 0; i < mRenderableLight->getNumShadows(); ++i) {
			viewMatrix = mRenderableLight->getShadowViewMatrix(i);
			projectionMatrix = mRenderableLight->getShadowProjectionMatrix(i);
			viewProjectionMatrix = projectionMatrix * viewMatrix;

			if (mRenderableLight->isPointLight()) {
				shadowTarget->attach(*mRenderableLight->getShadowMap(), graphics::FrameBufferAttachment::kDepth, 0, 0, static_cast<int>(i));
			}
			else {
				shadowTarget->attach(*mRenderableLight->getShadowMap(), graphics::FrameBufferAttachment::kDepth, 0, static_cast<int>(i));
			}

			mShadowUniformsUpdater.update(viewMatrix, projectionMatrix);

			mRenderersFrustum->updateFrustum(viewProjectionMatrix);
			for (const RenderQueueData& data : mTerrainRenderQueue) {
				mShadowTerrainRenderer->submit(*data.renderable, *data.pass);
			}
			for (const RenderQueueData& data : mMeshRenderQueue) {
				mShadowMeshRenderer->submit(*data.renderable, *data.pass);
			}

			mGraph.execute();
		}

		// Reset the viewport size to the previous one
		graphics::GraphicsOperations::setCullingMode(graphics::FaceMode::Front);
		graphics::GraphicsOperations::setViewport(0, 0, mWidth, mHeight);
	}


	void ShadowRenderSubGraph::endRender()
	{
		mRenderableLight = nullptr;
		setBindable(mShadowBindableIndex, nullptr);
	}


	void ShadowRenderSubGraph::clearQueues()
	{
		mTerrainRenderQueue.clear();
		mMeshRenderQueue.clear();
	}


	void ShadowRenderSubGraph::submitTerrain(graphics::Renderable& renderable, graphics::Pass& pass)
	{
		mTerrainRenderQueue.push_back({ &renderable, &pass });
	}


	void ShadowRenderSubGraph::submitMesh(graphics::Renderable& renderable, graphics::Pass& pass)
	{
		mMeshRenderQueue.push_back({ &renderable, &pass });
	}


	ShadowProxyRenderer::ShadowProxyRenderer(const std::string& name, ShadowRenderSubGraph& subGraph) :
		graphics::Renderer(name), mShadowRenderSubGraph(subGraph)
	{
		std::size_t targetIndex = dynamic_cast<graphics::BindableRNodeInput<graphics::FrameBuffer>*>( findInput("target") )->getBindableIndex();
		removeInput( findInput("target") );
		removeOutput( findOutput("target") );
		removeBindable(targetIndex);
	}

}
