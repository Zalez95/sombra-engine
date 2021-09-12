#include "se/graphics/FBClearNode.h"
#include "se/graphics/TextureUnitNode.h"
#include "se/graphics/ViewportResolutionNode.h"
#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/app/graphics/DeferredLightSubGraph.h"
#include "se/app/io/ShaderLoader.h"

namespace se::app {

	/**
	 * Class StartDLRenderNode, it's a RenderNode used for setting the Graphics
	 * state before rendering a light to the Stencil and Color buffers
	 */
	class DeferredLightSubGraph::StartDLRenderNode : public graphics::RenderNode
	{
	public:		// Functions
		/** Creates a new StartDLRenderNode
		 *
		 * @param	name the name of the new StartDLRenderNode */
		StartDLRenderNode(const std::string& name) : graphics::RenderNode(name)
		{
			addInput( std::make_unique<graphics::RNodeInput>("attach", this) );
			addOutput( std::make_unique<graphics::RNodeOutput>("attach", this) );
		};

		/** Class destructor */
		virtual ~StartDLRenderNode() = default;

		/** @copydoc graphics::RenderNode::execute() */
		virtual void execute() override
		{
			graphics::GraphicsOperations::setBlendEquation(graphics::BlendEquation::Add);
			graphics::GraphicsOperations::setBlendFunction(graphics::BlendFunction::One, graphics::BlendFunction::One);
			graphics::GraphicsOperations::setCullingMode(graphics::FaceMode::Front);
			graphics::GraphicsOperations::setDepthMask(false);
			graphics::GraphicsOperations::setStencilMask(true);
			graphics::GraphicsOperations::setOperation(graphics::Operation::StencilTest, true);
		};
	};


	/**
	 * Class EndDLRenderNode, it's a RenderNode used for resetting the Graphics
	 * state after rendering a light to the Stencil and Color buffers
	 */
	class DeferredLightSubGraph::EndDLRenderNode : public graphics::RenderNode
	{
	public:		// Functions
		/** Creates a new EndDLRenderNode
		 *
		 * @param	name the name of the new EndDLRenderNode */
		EndDLRenderNode(const std::string& name) : graphics::RenderNode(name)
		{
			addInput( std::make_unique<graphics::RNodeInput>("attach", this) );
		};

		/** Class destructor */
		virtual ~EndDLRenderNode() = default;

		/** @copydoc graphics::RenderNode::execute() */
		virtual void execute() override
		{
			graphics::GraphicsOperations::setOperation(graphics::Operation::StencilTest, false);
			graphics::GraphicsOperations::setStencilMask(false);
			graphics::GraphicsOperations::setDepthMask(true);
			graphics::GraphicsOperations::setCullingMode(graphics::FaceMode::Back);
			graphics::GraphicsOperations::setBlendFunction(graphics::BlendFunction::SourceAlpha, graphics::BlendFunction::OneMinusSourceAlpha);
		};
	};


	DeferredLightSubGraph::DeferredLightSubGraph(const std::string& name) : graphics::Renderer3D(name)
	{
		// Nodes
		auto resources = dynamic_cast<BindableRenderNode*>(mGraph.getNode("resources"));

		resources->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::FrameBuffer>>("lightTarget", resources, resources->addBindable()) );
		resources->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("position", resources, resources->addBindable()) );
		resources->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("normal", resources, resources->addBindable()) );
		resources->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("albedo", resources, resources->addBindable()) );
		resources->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("material", resources, resources->addBindable()) );
		resources->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("shadow", resources, mShadowBindableIndex) );

		auto shadowRenderSubGraph = std::make_unique<ShadowRenderSubGraph>("shadowRenderSubGraph");
		mShadowRenderSubGraph = shadowRenderSubGraph.get();

		auto texUnitNodeShadow = std::make_unique<graphics::TextureUnitNode>("texUnitNodeShadow", TexUnits::kShadow);
		texUnitNodeShadow->addOutput( std::make_unique<graphics::RNodeOutput>("attach", texUnitNodeShadow.get()) );

		auto startDLRenderNode = std::make_unique<StartDLRenderNode>("startDLRenderNode");

		auto stencilFBClear = std::make_unique<graphics::FBClearNode>("stencilFBClear", graphics::FrameBufferMask::Mask().set(graphics::FrameBufferMask::kStencil));
		stencilFBClear->addInput( std::make_unique<graphics::RNodeInput>("attach", stencilFBClear.get()) );

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
		colorRenderer->addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("shadow", colorRenderer.get(), colorRenderer->addBindable()) );
		colorRenderer->addOutput( std::make_unique<graphics::RNodeOutput>("attach", colorRenderer.get()) );
		mColorRenderer = colorRenderer.get();

		auto endDLRenderNode = std::make_unique<EndDLRenderNode>("endDLRenderNode");

		// Connections
		if (texUnitNodeShadow->findInput("input")->connect( shadowRenderSubGraph->findOutput("shadow") )
			&& startDLRenderNode->findInput("attach")->connect( texUnitNodeShadow->findOutput("attach") )
			&& stencilFBClear->findInput("target")->connect( resources->findOutput("lightTarget") )
			&& stencilFBClear->findInput("attach")->connect( startDLRenderNode->findOutput("attach") )
			&& stencilRenderer->findInput("target")->connect( stencilFBClear->findOutput("target") )
			&& colorRenderer->findInput("target")->connect( stencilRenderer->findOutput("target") )
			&& colorRenderer->findInput("position")->connect( resources->findOutput("position") )
			&& colorRenderer->findInput("normal")->connect( resources->findOutput("normal") )
			&& colorRenderer->findInput("albedo")->connect( resources->findOutput("albedo") )
			&& colorRenderer->findInput("material")->connect( resources->findOutput("material") )
			&& colorRenderer->findInput("shadow")->connect( texUnitNodeShadow->findOutput("output") )
			&& endDLRenderNode->findInput("attach")->connect( colorRenderer->findOutput("attach") )
			&& mGraph.addNode( std::move(shadowRenderSubGraph) )
			&& mGraph.addNode( std::move(texUnitNodeShadow) )
			&& mGraph.addNode( std::move(startDLRenderNode) )
			&& mGraph.addNode( std::move(stencilFBClear) )
			&& mGraph.addNode( std::move(stencilRenderer) )
			&& mGraph.addNode( std::move(colorRenderer) )
			&& mGraph.addNode( std::move(endDLRenderNode) )
		) {
			mGraph.prepareGraph();

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


	void DeferredLightSubGraph::sortQueue()
	{
		// Sort the render queue by Renderable
		std::sort(mLightsRenderQueue.begin(), mLightsRenderQueue.end());
	}


	void DeferredLightSubGraph::render()
	{
		for (RenderableLight* renderable : mLightsRenderQueue) {
			renderable->getRenderableMesh().submit();

			mShadowRenderSubGraph->startRender(*renderable);
			mGraph.execute();
			mShadowRenderSubGraph->endRender();
		}
	}


	void DeferredLightSubGraph::clearQueue()
	{
		mShadowRenderSubGraph->clearQueues();
		mLightsRenderQueue.clear();
	}


	void DeferredLightSubGraph::submitRenderable3D(graphics::Renderable3D& renderable, graphics::Pass&)
	{
		auto renderableLight = dynamic_cast<RenderableLight*>(&renderable);
		if (renderableLight) {
			mLightsRenderQueue.emplace_back(renderableLight);
		}
	}

}
