#include <cassert>
#include "se/graphics/3D/RendererMesh.h"
#include "se/graphics/core/Texture.h"
#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/graphics/FBClearNode.h"
#include "se/app/graphics/DeferredLightSubGraph.h"
#include "se/app/io/ShaderLoader.h"

namespace se::app {

	DeferredLightSubGraph::DeferredLightSubGraph(const std::string& name) :
		graphics::Renderer(name), mStencilRenderer(nullptr), mColorRenderer(nullptr)
	{
		// Graph
		auto resources = dynamic_cast<BindableRenderNode*>(mGraph.getNode("resources"));

		resources->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::FrameBuffer>>("lightTarget", resources, resources->addBindable()) );
		resources->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("position", resources, resources->addBindable()) );
		resources->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("normal", resources, resources->addBindable()) );
		resources->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("albedo", resources, resources->addBindable()) );
		resources->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("material", resources, resources->addBindable()) );

		auto stencilFBClear = std::make_unique<graphics::FBClearNode>("stencilFBClear", graphics::FrameBufferMask::Mask().set(graphics::FrameBufferMask::kStencil));

		auto stencilRenderer = std::make_unique<graphics::RendererMesh>("stencilRenderer");
		stencilRenderer->addBindable(std::make_shared<graphics::SetOperation>(graphics::Operation::DepthTest, true));
		stencilRenderer->addBindable(std::make_shared<graphics::SetOperation>(graphics::Operation::Culling, false));
		stencilRenderer->addBindable(std::make_shared<graphics::SetOperation>(graphics::Operation::Blending, false));
		stencilRenderer->addBindable(std::make_shared<graphics::BindableOperation>([]() {
			graphics::GraphicsOperations::setColorMask(false, false, false, false);
			graphics::GraphicsOperations::setStencilFunction(graphics::StencilFunction::Always, 0, 0);
			graphics::GraphicsOperations::setStencilAction(
				graphics::FaceMode::Back,
				graphics::StencilAction::Keep, graphics::StencilAction::IncrementAndWrap, graphics::StencilAction::Keep
			);
			graphics::GraphicsOperations::setStencilAction(
				graphics::FaceMode::Front,
				graphics::StencilAction::Keep, graphics::StencilAction::DecrementAndWrap, graphics::StencilAction::Keep
			);
		}));
		mStencilRenderer = stencilRenderer.get();

		auto colorRenderer = std::make_unique<graphics::RendererMesh>("colorRenderer");
		colorRenderer->addBindable(std::make_shared<graphics::SetOperation>(graphics::Operation::DepthTest, false));
		colorRenderer->addBindable(std::make_shared<graphics::SetOperation>(graphics::Operation::Culling, true));
		colorRenderer->addBindable(std::make_shared<graphics::SetOperation>(graphics::Operation::Blending, true));
		colorRenderer->addBindable(std::make_shared<graphics::BindableOperation>([]() {
			graphics::GraphicsOperations::setColorMask(true, true, true, true);
			graphics::GraphicsOperations::setStencilFunction(graphics::StencilFunction::NotEqual, 0, 0xFF);
		}));
		colorRenderer->addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("position", colorRenderer.get(), colorRenderer->addBindable()) );
		colorRenderer->addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("normal", colorRenderer.get(), colorRenderer->addBindable()) );
		colorRenderer->addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("albedo", colorRenderer.get(), colorRenderer->addBindable()) );
		colorRenderer->addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("material", colorRenderer.get(), colorRenderer->addBindable()) );
		mColorRenderer = colorRenderer.get();

		if (stencilFBClear->findInput("input")->connect( resources->findOutput("lightTarget") )
			&& stencilRenderer->findInput("target")->connect( stencilFBClear->findOutput("output") )
			&& colorRenderer->findInput("target")->connect( stencilRenderer->findOutput("target") )
			&& colorRenderer->findInput("position")->connect( resources->findOutput("position") )
			&& colorRenderer->findInput("normal")->connect( resources->findOutput("normal") )
			&& colorRenderer->findInput("albedo")->connect( resources->findOutput("albedo") )
			&& colorRenderer->findInput("material")->connect( resources->findOutput("material") )
			&& mGraph.addNode( std::move(stencilFBClear) )
			&& mGraph.addNode( std::move(stencilRenderer) )
			&& mGraph.addNode( std::move(colorRenderer) )
		) {
			mGraph.prepareGraph();

			addBindable(std::make_shared<graphics::SetDepthMask>(false));

			mTargetBindableIndex = dynamic_cast<graphics::BindableRNodeInput<graphics::FrameBuffer>*>( findInput("target") )->getBindableIndex();
			mPositionTextureBindableIndex = addBindable();
			addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("position", this, mPositionTextureBindableIndex) );
			mNormalTextureBindableIndex = addBindable();
			addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("normal", this, mNormalTextureBindableIndex) );
			mAlbedoTextureBindableIndex = addBindable();
			addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("albedo", this, mAlbedoTextureBindableIndex) );
			mMaterialTextureBindableIndex = addBindable();
			addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("material", this, mMaterialTextureBindableIndex) );
		}
	}


	void DeferredLightSubGraph::setBindable(std::size_t bindableIndex, const BindableSPtr& bindable)
	{
		if (bindableIndex == mTargetBindableIndex) {
			auto resources = dynamic_cast<graphics::BindableRenderNode*>(mGraph.getNode("resources"));
			auto output = dynamic_cast<graphics::BindableRNodeOutput<graphics::FrameBuffer>*>( resources->findOutput("lightTarget") );
			resources->setBindable(output->getBindableIndex(), bindable);
		}
		else if (bindableIndex == mPositionTextureBindableIndex) {
			auto resources = dynamic_cast<graphics::BindableRenderNode*>(mGraph.getNode("resources"));
			auto output = dynamic_cast<graphics::BindableRNodeOutput<graphics::Texture>*>( resources->findOutput("position") );
			resources->setBindable(output->getBindableIndex(), bindable);
		}
		else if (bindableIndex == mNormalTextureBindableIndex) {
			auto resources = dynamic_cast<graphics::BindableRenderNode*>(mGraph.getNode("resources"));
			auto output = dynamic_cast<graphics::BindableRNodeOutput<graphics::Texture>*>( resources->findOutput("normal") );
			resources->setBindable(output->getBindableIndex(), bindable);
		}
		else if (bindableIndex == mAlbedoTextureBindableIndex) {
			auto resources = dynamic_cast<graphics::BindableRenderNode*>(mGraph.getNode("resources"));
			auto output = dynamic_cast<graphics::BindableRNodeOutput<graphics::Texture>*>( resources->findOutput("albedo") );
			resources->setBindable(output->getBindableIndex(), bindable);
		}
		else if (bindableIndex == mMaterialTextureBindableIndex) {
			auto resources = dynamic_cast<graphics::BindableRenderNode*>(mGraph.getNode("resources"));
			auto output = dynamic_cast<graphics::BindableRNodeOutput<graphics::Texture>*>( resources->findOutput("material") );
			resources->setBindable(output->getBindableIndex(), bindable);
		}

		BindableRenderNode::setBindable(bindableIndex, bindable);
	}


	void DeferredLightSubGraph::submit(graphics::Renderable&, graphics::Pass&)
	{
		assert("You can't submit Renderables to this node");
	}


	void DeferredLightSubGraph::submitStencil(graphics::Renderable& renderable, graphics::Pass& pass)
	{
		// TODO: frustum
		mLightsRenderQueue.push_back({ &renderable, &pass, true });
	}


	void DeferredLightSubGraph::submitColor(graphics::Renderable& renderable, graphics::Pass& pass)
	{
		// TODO: frustum
		mLightsRenderQueue.push_back({ &renderable, &pass, false });
	}


	void DeferredLightSubGraph::sortQueue()
	{
		// Sort the render queue by Renderable
		std::sort(
			mLightsRenderQueue.begin(), mLightsRenderQueue.end(),
			[](const RenderQueueData& lhs, const RenderQueueData& rhs) { return lhs.renderable < rhs.renderable; }
		);

		// Sort again so the stencil pass is always first to the color one
		for (auto it = mLightsRenderQueue.begin(); it != mLightsRenderQueue.end();) {
			auto it2 = std::next(it);
			if (it->renderable == it2->renderable) {
				if (it->stencil) {
					if (it2->stencil) {
						it = mLightsRenderQueue.erase(it);
					}
					else {
						it += 2;
					}
				}
				else {
					if (it2->stencil) {
						std::swap(*it, *it2);
						it += 2;
					}
					else {
						it = mLightsRenderQueue.erase(it);
					}
				}
			}
			else {
				it = mLightsRenderQueue.erase(it);
			}
		}
	}


	void DeferredLightSubGraph::render()
	{
		graphics::GraphicsOperations::setCullingMode(graphics::FaceMode::Front);
		graphics::GraphicsOperations::setStencilMask(true);
		graphics::GraphicsOperations::setOperation(graphics::Operation::StencilTest, true);
		graphics::GraphicsOperations::setBlendEquation(graphics::BlendEquation::Add);
		graphics::GraphicsOperations::setBlendFunction(graphics::BlendFunction::One, graphics::BlendFunction::One);

		for (std::size_t i = 0; i < mLightsRenderQueue.size(); i += 2) {
			mStencilRenderer->submit(*mLightsRenderQueue[i].renderable, *mLightsRenderQueue[i].pass);
			mColorRenderer->submit(*mLightsRenderQueue[i+1].renderable, *mLightsRenderQueue[i+1].pass);
			mGraph.execute();
		}

		graphics::GraphicsOperations::setBlendFunction(graphics::BlendFunction::SourceAlpha, graphics::BlendFunction::OneMinusSourceAlpha);
		graphics::GraphicsOperations::setOperation(graphics::Operation::StencilTest, false);
		graphics::GraphicsOperations::setStencilMask(false);
		graphics::GraphicsOperations::setCullingMode(graphics::FaceMode::Back);
	}


	void DeferredLightSubGraph::clearQueue()
	{
		mLightsRenderQueue.clear();
	}


	DeferredLightProxyRenderer::DeferredLightProxyRenderer(const std::string& name, DeferredLightSubGraph& subGraph) :
		Renderer(name), mDeferredLightSubGraph(subGraph)
	{
		addInput( std::make_unique<graphics::RNodeInput>("attach", this) );
		addOutput( std::make_unique<graphics::RNodeOutput>("attach", this) );
	}

}
