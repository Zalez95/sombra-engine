#include "se/graphics/core/Program.h"
#include "se/graphics/core/Texture.h"
#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/UniformVariable.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/graphics/ViewportResolutionNode.h"
#include "se/app/io/ShaderLoader.h"
#include "ShadowRenderSubGraph.h"

namespace se::app {

	using namespace graphics;


	/**
	 * Class StartShadowNode, it's the ViewportResolutionNode that will be
	 * executed before rendering each Shadow for setting the viewport size,
	 * uniform variables and other operations
	 */
	class ShadowRenderSubGraph::StartShadowNode : public ViewportResolutionNode
	{
	private:	// Attributes
		/** The ShadowRenderSubGraph that owns the current StartShadowNode */
		ShadowRenderSubGraph* mParent;

		/** The index of the Shadow to render in the ShadowRenderSubGraph */
		std::size_t mShadowIndex;

	public:		// Functions
		/** Creates a new StartShadowNode
		 *
		 * @param	name the name of the new StartShadowNode
		 * @param	parent the ShadowRenderSubGraph that owns the current
		 *			StartShadowNode
		 * @param	shadowIndex the index of the Shadow to render in the
		 *			ShadowRenderSubGraph */
		StartShadowNode(const std::string& name, ShadowRenderSubGraph* parent, std::size_t shadowIndex) :
			ViewportResolutionNode(name), mParent(parent), mShadowIndex(shadowIndex) {};

		/** @copydoc RenderNode::execute() */
		virtual void execute() override
		{
			glm::mat4 viewMatrix = mParent->mShadows[mShadowIndex].viewMatrix;
			glm::mat4 projectionMatrix = mParent->mShadows[mShadowIndex].projectionMatrix;
			mParent->mShadowUniformsUpdater->update(viewMatrix, projectionMatrix);

			GraphicsOperations::setCullingMode(FaceMode::Front);
			ViewportResolutionNode::execute();
		};
	};


	/**
	 * Class EndShadowNode, it's the ViewportResolutionNode used for clearing
	 * the state changed in @see StartShadowNode after rendering the Shadows
	 */
	class ShadowRenderSubGraph::EndShadowNode : public ViewportResolutionNode
	{
	public:		// Functions
		/** Creates a new EndShadowNode
		 *
		 * @param	name the name of the new EndShadowNode */
		EndShadowNode(const std::string& name) : ViewportResolutionNode(name) {};

		/** @copydoc RenderNode::execute() */
		virtual void execute() override
		{
			ViewportResolutionNode::execute();
			GraphicsOperations::setCullingMode(FaceMode::Back);
		};
	};


	bool ShadowUniformsUpdater::shouldAddUniforms(const PassSPtr& pass) const
	{
		return std::find(mRenderers.begin(), mRenderers.end(), &pass->getRenderer()) != mRenderers.end();
	}


	ShadowRenderSubGraph::ShadowRenderSubGraph(const char* name, Repository& repository) :
		BindableRenderNode(name), mMergeShadowsNode(nullptr), mShadowUniformsUpdater(nullptr)
	{
		mShadowUniformsUpdater = new ShadowUniformsUpdater("uViewMatrix", "uProjectionMatrix");

		// Graph
		auto resources = dynamic_cast<BindableRenderNode*>(mGraph.getNode("resources"));
		auto endShadowsNode = std::make_unique<EndShadowNode>("endShadows");
		auto mergeShadowsNode = std::make_unique<MergeShadowsNode>("mergeShadows", repository);
		mMergeShadowsNode = mergeShadowsNode.get();

		auto iShadowTargetBindable = resources->addBindable();
		resources->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("shadowTarget", resources, iShadowTargetBindable) );

		auto iPositionTextureBindable = resources->addBindable();
		auto iNormalTextureBindable = resources->addBindable();
		resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("position", resources, iPositionTextureBindable) );
		resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("normal", resources, iNormalTextureBindable) );

		if (mergeShadowsNode->findInput("target")->connect( resources->findOutput("shadowTarget") )
			&& mergeShadowsNode->findInput("position")->connect( resources->findOutput("position") )
			&& mergeShadowsNode->findInput("normal")->connect( resources->findOutput("normal") )
			&& mergeShadowsNode->findInput("attach")->connect( endShadowsNode->findOutput("attach") )
			&& mGraph.addNode( std::move(endShadowsNode) )
			&& mGraph.addNode( std::move(mergeShadowsNode) )
		) {
			mTargetBindableIndex = addBindable();
			addInput(std::make_unique<BindableRNodeInput<FrameBuffer>>("target", this, mTargetBindableIndex));
			addOutput(std::make_unique<BindableRNodeOutput<FrameBuffer>>("target", this, mTargetBindableIndex));

			mPositionTextureBindableIndex = addBindable();
			mNormalTextureBindableIndex = addBindable();
			addInput(std::make_unique<BindableRNodeInput<Texture>>("position", this, mPositionTextureBindableIndex));
			addInput(std::make_unique<BindableRNodeInput<Texture>>("normal", this, mNormalTextureBindableIndex));
		}
	}


	ShadowRenderSubGraph::~ShadowRenderSubGraph()
	{
		delete mShadowUniformsUpdater;
	}


	void ShadowRenderSubGraph::execute()
	{
		mGraph.execute();
	}


	void ShadowRenderSubGraph::setBindable(std::size_t bindableIndex, const BindableSPtr& bindable)
	{
		if (bindableIndex == mTargetBindableIndex) {
			auto resources = dynamic_cast<BindableRenderNode*>(mGraph.getNode("resources"));
			auto output = dynamic_cast<BindableRNodeOutput<FrameBuffer>*>( resources->findOutput("shadowTarget") );
			resources->setBindable(output->getBindableIndex(), bindable);
		}
		else if (bindableIndex == mPositionTextureBindableIndex) {
			auto resources = dynamic_cast<BindableRenderNode*>(mGraph.getNode("resources"));
			auto output = dynamic_cast<BindableRNodeOutput<Texture>*>( resources->findOutput("position") );
			resources->setBindable(output->getBindableIndex(), bindable);
		}
		else if (bindableIndex == mNormalTextureBindableIndex) {
			auto resources = dynamic_cast<BindableRenderNode*>(mGraph.getNode("resources"));
			auto output = dynamic_cast<BindableRNodeOutput<Texture>*>( resources->findOutput("normal") );
			resources->setBindable(output->getBindableIndex(), bindable);
		}

		BindableRenderNode::setBindable(bindableIndex, bindable);
	}


	void ShadowRenderSubGraph::setInvCameraViewProjectionMatrix(const glm::mat4& invVPMatrix)
	{
		mMergeShadowsNode->setInvCameraVPMatrix(invVPMatrix);
	}


	void ShadowRenderSubGraph::setCameraResolution(std::size_t width, std::size_t height)
	{
		auto endShadowNode = dynamic_cast<EndShadowNode*>(mGraph.getNode("endShadows"));
		endShadowNode->setViewportSize(0, 0, width, height);
	}


	std::size_t ShadowRenderSubGraph::addShadow(std::size_t resolution, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		auto itShadow = std::find_if(mShadows.begin(), mShadows.end(), [](const Shadow& shadow) { return !shadow.active; });
		if (itShadow == mShadows.end()) {
			return -1;
		}

		itShadow->active = true;
		std::size_t shadowIndex = std::distance(mShadows.begin(), itShadow);
		std::string shadowIndexStr = std::to_string(shadowIndex);

		// Resources
		auto resources = dynamic_cast<BindableRenderNode*>(mGraph.getNode("resources"));
		auto endShadowNode = dynamic_cast<EndShadowNode*>(mGraph.getNode("endShadows"));

		auto shadowBuffer = std::make_shared<FrameBuffer>();
		shadowBuffer->setColorBuffer(false);
		auto iFrameBufferBindable = resources->addBindable(shadowBuffer);
		resources->addOutput(std::make_unique<BindableRNodeOutput<FrameBuffer>>(
			"shadowBuffer" + shadowIndexStr, resources, iFrameBufferBindable
		));

		auto shadowTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		shadowTexture->setImage(nullptr, TypeId::Float, ColorFormat::Depth, ColorFormat::Depth, 1, 1)
			.setWrapping(TextureWrap::ClampToBorder, TextureWrap::ClampToBorder)
			.setFiltering(TextureFilter::Nearest, TextureFilter::Nearest)
			.setBorderColor(1.0f, 1.0f, 1.0f, 1.0f)
			.setTextureUnit(static_cast<int>(shadowIndex));
		shadowBuffer->attach(*shadowTexture, FrameBufferAttachment::kDepth);
		auto iShadowTextureBindable = resources->addBindable(shadowTexture);
		resources->addOutput(std::make_unique<BindableRNodeOutput<Texture>>(
			"shadowTexture" + shadowIndexStr, resources, iShadowTextureBindable
		));

		endShadowNode->addInput(std::make_unique<RNodeInput>("attach" + shadowIndexStr, endShadowNode));

		// Nodes
		auto startShadowNode = std::make_unique<StartShadowNode>("startShadow" + shadowIndexStr, this, shadowIndex);
		itShadow->startNode = startShadowNode.get();

		auto shadowFBClear = std::make_unique<FBClearNode>(
			"shadowFBClear" + std::to_string(shadowIndex),
			FrameBufferMask::Mask().set(FrameBufferMask::kDepth)
		);
		shadowFBClear->addInput( std::make_unique<RNodeInput>("attach", shadowFBClear.get()) );
		itShadow->clearFB = shadowFBClear.get();

		auto shadowRendererTerrain = std::make_unique<RendererTerrain>("shadowRendererTerrain" + shadowIndexStr);
		itShadow->rendererTerrain = shadowRendererTerrain.get();

		auto shadowRendererMesh = std::make_unique<RendererMesh>("shadowRendererMesh" + shadowIndexStr);
		shadowRendererMesh->addOutput( std::make_unique<RNodeOutput>("attach", shadowRendererMesh.get()) );
		itShadow->rendererMesh = shadowRendererMesh.get();

		auto frustum = std::make_shared<FrustumFilter>();
		shadowRendererMesh->addFilter(frustum);
		shadowRendererTerrain->addFilter(frustum);
		itShadow->frustum = frustum.get();

		if (shadowFBClear->findInput("input")->connect( resources->findOutput("shadowBuffer" + shadowIndexStr) )
			&& shadowFBClear->findInput("attach")->connect( startShadowNode->findOutput("attach") )
			&& shadowRendererTerrain->findInput("target")->connect( shadowFBClear->findOutput("output") )
			&& shadowRendererMesh->findInput("target")->connect( shadowRendererTerrain->findOutput("target") )
			&& endShadowNode->findInput("attach" + shadowIndexStr)->connect( shadowRendererMesh->findOutput("attach") )
			&& mMergeShadowsNode->findInput("shadowTexture" + shadowIndexStr)->connect( resources->findOutput("shadowTexture" + shadowIndexStr) )
			&& mGraph.addNode( std::move(startShadowNode) )
			&& mGraph.addNode( std::move(shadowFBClear) )
			&& mGraph.addNode( std::move(shadowRendererTerrain) )
			&& mGraph.addNode( std::move(shadowRendererMesh) )
		) {
			mGraph.prepareGraph();

			mMergeShadowsNode->enableShadow(shadowIndex, true);
			setShadowVPMatrix(shadowIndex, viewMatrix, projectionMatrix);
			setShadowResolution(shadowIndex, resolution);

			return shadowIndex;
		}

		removeShadow(shadowIndex);
		return -1;
	}


	void ShadowRenderSubGraph::setShadowVPMatrix(std::size_t shadowIndex, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		glm::mat4 vpMatrix = projectionMatrix * viewMatrix;

		mShadows[shadowIndex].viewMatrix = viewMatrix;
		mShadows[shadowIndex].projectionMatrix = projectionMatrix;
		mShadows[shadowIndex].frustum->updateFrustum(vpMatrix);
		mMergeShadowsNode->setShadowVPMatrix(shadowIndex, vpMatrix);
	}


	void ShadowRenderSubGraph::setShadowResolution(std::size_t shadowIndex, std::size_t resolution)
	{
		std::string shadowIndexStr = std::to_string(shadowIndex);

		auto resources = dynamic_cast<BindableRenderNode*>(mGraph.getNode("resources"));
		auto shadowTextureOutput = dynamic_cast<BindableRNodeOutput<Texture>*>( resources->findOutput("shadowTexture" + shadowIndexStr) );
		auto shadowTexture = std::dynamic_pointer_cast<Texture>(resources->getBindable(shadowTextureOutput->getBindableIndex()));
		shadowTexture->setImage(nullptr, TypeId::Float, ColorFormat::Depth, ColorFormat::Depth, resolution, resolution);

		mShadows[shadowIndex].startNode->setViewportSize(0, 0, resolution, resolution);
	}


	void ShadowRenderSubGraph::removeShadow(std::size_t shadowIndex)
	{
		std::string shadowIndexStr = std::to_string(shadowIndex);

		mMergeShadowsNode->enableShadow(shadowIndex, false);

		// Nodes
		mShadows[shadowIndex].startNode->disconnect();
		mGraph.removeNode(mShadows[shadowIndex].startNode);
		mShadows[shadowIndex].startNode = nullptr;

		mShadows[shadowIndex].clearFB->disconnect();
		mGraph.removeNode(mShadows[shadowIndex].clearFB);
		mShadows[shadowIndex].clearFB = nullptr;

		mShadows[shadowIndex].rendererTerrain->disconnect();
		mGraph.removeNode(mShadows[shadowIndex].rendererTerrain);
		mShadows[shadowIndex].rendererTerrain = nullptr;

		mShadows[shadowIndex].rendererMesh->disconnect();
		mGraph.removeNode(mShadows[shadowIndex].rendererMesh);
		mShadows[shadowIndex].rendererMesh = nullptr;

		mShadows[shadowIndex].frustum = nullptr;

		// Resources
		auto resources = dynamic_cast<BindableRenderNode*>(mGraph.getNode("resources"));
		auto endShadowNode = dynamic_cast<EndShadowNode*>(mGraph.getNode("endShadows"));

		auto shadowBufferOutput = dynamic_cast<BindableRNodeOutput<FrameBuffer>*>( resources->findOutput("shadowBuffer" + shadowIndexStr) );
		auto iFrameBufferBindable = shadowBufferOutput->getBindableIndex();
		shadowBufferOutput->disconnect();
		resources->removeOutput(shadowBufferOutput);
		resources->setBindable(iFrameBufferBindable, nullptr);

		auto shadowTextureOutput = dynamic_cast<BindableRNodeOutput<Texture>*>( resources->findOutput("shadowTexture" + shadowIndexStr) );
		auto iShadowTextureBindable = shadowTextureOutput->getBindableIndex();
		shadowTextureOutput->disconnect();
		resources->removeOutput(shadowTextureOutput);
		resources->setBindable(iShadowTextureBindable, nullptr);

		auto attachInput = endShadowNode->findInput("attach" + shadowIndexStr);
		endShadowNode->removeInput(attachInput);

		mShadows[shadowIndex].active = false;

		mGraph.prepareGraph();
	}


	void ShadowRenderSubGraph::submitTerrain(Renderable& renderable, Pass& pass)
	{
		for (Shadow& shadow : mShadows) {
			if (shadow.active) {
				shadow.rendererTerrain->submit(renderable, pass);
			}
		}
	}


	void ShadowRenderSubGraph::submitMesh(Renderable& renderable, Pass& pass)
	{
		for (Shadow& shadow : mShadows) {
			if (shadow.active) {
				shadow.rendererMesh->submit(renderable, pass);
			}
		}
	}


	ShadowProxyRenderer::ShadowProxyRenderer(const std::string& name, ShadowRenderSubGraph& subGraph) :
		Renderer(name), mShadowRenderSubGraph(subGraph)
	{
		addInput( std::make_unique<RNodeInput>("attach", this) );
		addOutput( std::make_unique<RNodeOutput>("attach", this) );
	}

}
