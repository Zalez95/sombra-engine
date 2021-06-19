#include "se/utils/Log.h"
#include "se/graphics/FBClearNode.h"
#include "se/graphics/FBCopyNode.h"
#include "se/graphics/TextureUnitNode.h"
#include "se/graphics/ViewportResolutionNode.h"
#include "se/graphics/2D/Renderer2D.h"
#include "se/graphics/3D/RendererMesh.h"
#include "se/graphics/3D/RendererTerrain.h"
#include "se/graphics/3D/RendererParticles.h"
#include "se/graphics/3D/FrustumFilter.h"
#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/UniformBlock.h"
#include "se/graphics/core/UniformVariable.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/app/io/MeshLoader.h"
#include "se/app/io/ShaderLoader.h"
#include "se/app/graphics/GaussianBlurNode.h"
#include "se/app/graphics/TextureUtils.h"
#include "se/app/graphics/DeferredLightRenderer.h"
#include "se/app/graphics/AppRenderGraph.h"

using namespace se::graphics;
using namespace std::string_literals;

namespace se::app {

	class StartShadowNode : public ViewportResolutionNode
	{
	public:
		StartShadowNode(const std::string& name) : ViewportResolutionNode(name) {};
		virtual ~StartShadowNode() = default;

		virtual void execute() override
		{
			graphics::SetOperation(graphics::Operation::DepthTest).bind();
			graphics::GraphicsOperations::setCullingMode(graphics::FaceMode::Front);
			ViewportResolutionNode::execute();
		};
	};


	class EndShadowNode : public ViewportResolutionNode
	{
	public:
		EndShadowNode(const std::string& name) : ViewportResolutionNode(name) {};
		virtual ~EndShadowNode() = default;

		virtual void execute() override
		{
			ViewportResolutionNode::execute();
			graphics::GraphicsOperations::setCullingMode(graphics::FaceMode::Back);
			graphics::SetOperation(graphics::Operation::DepthTest).unbind();
		};
	};


	class CombineNode : public BindableRenderNode
	{
	public:
		static constexpr int kColor0 = 0;
		static constexpr int kColor1 = 1;
	private:
		Repository::ResourceRef<Mesh> mPlane;
		Repository::ResourceRef<Program> mProgram;

	public:
		CombineNode(const std::string& name, Repository& repository) :
			BindableRenderNode(name)
		{
			auto iTargetBindable = addBindable();
			addInput( std::make_unique<BindableRNodeInput<FrameBuffer>>("target", this, iTargetBindable) );
			addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("target", this, iTargetBindable) );

			addInput( std::make_unique<BindableRNodeInput<Texture>>("color0", this, addBindable()) );
			addInput( std::make_unique<BindableRNodeInput<Texture>>("color1", this, addBindable()) );

			mPlane = repository.findByName<Mesh>("plane");

			mProgram = repository.findByName<Program>("fragmentCombineHDR");
			if (!mProgram) {
				std::shared_ptr<Program> program;
				auto result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentCombineHDR.glsl", program);
				if (!result) {
					SOMBRA_ERROR_LOG << result.description();
					return;
				}
				mProgram = repository.insert(std::move(program), "programCombineHDR");
			}

			addBindable(mProgram.get());
			addBindable(std::make_shared<UniformVariableValue<glm::mat4>>("uModelMatrix", mProgram.get(), glm::mat4(1.0f)));
			addBindable(std::make_shared<UniformVariableValue<glm::mat4>>("uViewMatrix", mProgram.get(), glm::mat4(1.0f)));
			addBindable(std::make_shared<UniformVariableValue<glm::mat4>>("uProjectionMatrix", mProgram.get(), glm::mat4(1.0f)));
			addBindable(std::make_shared<UniformVariableValue<int>>("uColor0", mProgram.get(), kColor0));
			addBindable(std::make_shared<UniformVariableValue<int>>("uColor1", mProgram.get(), kColor1));
		};

		virtual void execute() override
		{
			bind();
			mPlane->bind();
			graphics::GraphicsOperations::drawIndexedInstanced(
				graphics::PrimitiveType::Triangle,
				mPlane->getIBO().getIndexCount(), mPlane->getIBO().getIndexType()
			);
		};
	};


	AppRenderGraph::AppRenderGraph(Repository& repository, const ShadowData& shadowData, std::size_t width, std::size_t height)
	{
		if (!addResources(repository, shadowData, width, height)) {
			throw std::runtime_error("Failed to add resources");
		}
		if (!addNodes(repository, width, height)) {
			throw std::runtime_error("Failed to add nodes");
		}
		prepareGraph();
	}


	void AppRenderGraph::setResolution(std::size_t width, std::size_t height)
	{
		auto resources = getNode("resources");
		auto zBufferCopy = dynamic_cast<FBCopyNode*>(getNode("zBufferCopy"));
		auto hBlurNode = dynamic_cast<GaussianBlurNode*>(getNode("hBlurNode"));
		auto vBlurNode = dynamic_cast<GaussianBlurNode*>(getNode("vBlurNode"));
		auto endShadowNode = dynamic_cast<EndShadowNode*>(getNode("endShadow"));
		zBufferCopy->setDimensions1(0, 0, width, height).setDimensions2(0, 0, width, height);
		hBlurNode->setTextureDimensions(width, height);
		vBlurNode->setTextureDimensions(width, height);
		endShadowNode->setViewportSize(0, 0, width, height);

		auto zTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("zTexture"))->getTBindable();
		auto positionTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("positionTexture"))->getTBindable();
		auto normalTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("normalTexture"))->getTBindable();
		auto albedoTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("albedoTexture"))->getTBindable();
		auto materialTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("materialTexture"))->getTBindable();
		auto emissiveTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("emissiveTexture"))->getTBindable();
		auto depthTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("depthTexture"))->getTBindable();
		auto colorTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("colorTexture"))->getTBindable();
		auto brightTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("brightTexture"))->getTBindable();
		zTexture->setImage(nullptr, TypeId::Float, ColorFormat::Depth, ColorFormat::Depth24, width, height);
		positionTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGB, ColorFormat::RGB16f, width, height);
		normalTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGB, ColorFormat::RGB16f, width, height);
		albedoTexture->setImage(nullptr, TypeId::UnsignedByte, ColorFormat::RGB, ColorFormat::RGB, width, height);
		materialTexture->setImage(nullptr, TypeId::UnsignedByte, ColorFormat::RGB, ColorFormat::RGB, width, height);
		emissiveTexture->setImage(nullptr, TypeId::UnsignedByte, ColorFormat::RGB, ColorFormat::RGB, width, height);
		depthTexture->setImage(nullptr, TypeId::Float, ColorFormat::Depth, ColorFormat::Depth24, width, height);
		colorTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGBA, ColorFormat::RGBA16f, width, height);
		brightTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGBA, ColorFormat::RGBA16f, width, height);
	}

// Private functions
	bool AppRenderGraph::addResources(Repository& repository, const ShadowData& shadowData, std::size_t width, std::size_t height)
	{
		auto resources = dynamic_cast<BindableRenderNode*>(getNode("resources"));

		RawMesh planeRawMesh;
		planeRawMesh.positions = { {-1.0f,-1.0f, 0.0f }, { 1.0f,-1.0f, 0.0f }, {-1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f } };
		planeRawMesh.indices = { 0, 1, 2, 1, 3, 2, };
		auto planeMesh = std::make_shared<Mesh>(MeshLoader::createGraphicsMesh(planeRawMesh));
		repository.insert(std::move(planeMesh), "plane").setFakeUser();

		auto irradianceTextureResource = resources->addBindable();
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("irradianceTexture", resources, irradianceTextureResource) )) {
			return false;
		}

		auto prefilterTextureResource = resources->addBindable();
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("prefilterTexture", resources, prefilterTextureResource) )) {
			return false;
		}

		auto brdfTexture = TextureUtils::precomputeBRDF(512);
		if (!brdfTexture) {
			return false;
		}

		brdfTexture->setTextureUnit(DeferredLightRenderer::TexUnits::kBRDFMap);
		auto iBRDFTextureResource = resources->addBindable(brdfTexture);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("brdfTexture", resources, iBRDFTextureResource) )) {
			return false;
		}

		auto shadowBuffer = std::make_shared<FrameBuffer>();
		shadowBuffer->setColorBuffer(false);
		auto iShadowBufferResource = resources->addBindable(shadowBuffer);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("shadowBuffer", resources, iShadowBufferResource) )) {
			return false;
		}

		auto shadowTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		shadowTexture->setImage(nullptr, TypeId::Float, ColorFormat::Depth, ColorFormat::Depth, shadowData.resolution, shadowData.resolution)
			.setWrapping(TextureWrap::ClampToBorder, TextureWrap::ClampToBorder)
			.setBorderColor(1.0f, 1.0f, 1.0f, 1.0f)
			.setFiltering(TextureFilter::Nearest, TextureFilter::Nearest);
		shadowBuffer->attach(*shadowTexture, FrameBufferAttachment::kDepth);
		auto iShadowTextureResource = resources->addBindable(shadowTexture);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("shadowTexture", resources, iShadowTextureResource) )) {
			return false;
		}

		auto gBuffer = std::make_shared<FrameBuffer>();
		auto iGBufferResource = resources->addBindable(gBuffer);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("gBuffer", resources, iGBufferResource) )) {
			return false;
		}

		auto zTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		zTexture->setImage(nullptr, TypeId::Float, ColorFormat::Depth, ColorFormat::Depth24, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		gBuffer->attach(*zTexture, FrameBufferAttachment::kDepth);
		auto iZTextureResource = resources->addBindable(zTexture);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("zTexture", resources, iZTextureResource) )) {
			return false;
		}

		auto positionTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		positionTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGB, ColorFormat::RGB16f, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		gBuffer->attach(*positionTexture, FrameBufferAttachment::kColor0);
		auto iPositionTextureResource = resources->addBindable(positionTexture);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("positionTexture", resources, iPositionTextureResource) )) {
			return false;
		}

		auto normalTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		normalTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGB, ColorFormat::RGB16f, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		gBuffer->attach(*normalTexture, FrameBufferAttachment::kColor0 + 1);
		auto iNormalTextureResource = resources->addBindable(normalTexture);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("normalTexture", resources, iNormalTextureResource) )) {
			return false;
		}

		auto albedoTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		albedoTexture->setImage(nullptr, TypeId::UnsignedByte, ColorFormat::RGB, ColorFormat::RGB, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		gBuffer->attach(*albedoTexture, FrameBufferAttachment::kColor0 + 2);
		auto iAlbedoTextureResource = resources->addBindable(albedoTexture);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("albedoTexture", resources, iAlbedoTextureResource) )) {
			return false;
		}

		auto materialTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		materialTexture->setImage(nullptr, TypeId::UnsignedByte, ColorFormat::RGB, ColorFormat::RGB, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		gBuffer->attach(*materialTexture, FrameBufferAttachment::kColor0 + 3);
		auto iMaterialTextureResource = resources->addBindable(materialTexture);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("materialTexture", resources, iMaterialTextureResource) )) {
			return false;
		}

		auto emissiveTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		emissiveTexture->setImage(nullptr, TypeId::UnsignedByte, ColorFormat::RGB, ColorFormat::RGB, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		gBuffer->attach(*emissiveTexture, FrameBufferAttachment::kColor0 + 4);
		auto iEmissiveTextureResource = resources->addBindable(emissiveTexture);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("emissiveTexture", resources, iEmissiveTextureResource) )) {
			return false;
		}

		auto deferredBuffer = std::make_shared<FrameBuffer>();
		auto iDeferredBufferResource = resources->addBindable(deferredBuffer);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("deferredBuffer", resources, iDeferredBufferResource) )) {
			return false;
		}

		auto depthTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		depthTexture->setImage(nullptr, TypeId::Float, ColorFormat::Depth, ColorFormat::Depth24, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		deferredBuffer->attach(*depthTexture, FrameBufferAttachment::kDepth);
		auto iDepthTextureResource = resources->addBindable(depthTexture);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("depthTexture", resources, iDepthTextureResource) )) {
			return false;
		}

		auto colorTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		colorTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGBA, ColorFormat::RGBA16f, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		deferredBuffer->attach(*colorTexture, FrameBufferAttachment::kColor0);
		auto iColorTextureResource = resources->addBindable(colorTexture);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("colorTexture", resources, iColorTextureResource) )) {
			return false;
		}

		auto brightTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		brightTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGBA, ColorFormat::RGBA16f, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		deferredBuffer->attach(*brightTexture, FrameBufferAttachment::kColor0 + 1);
		auto iBrightTextureResource = resources->addBindable(brightTexture);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("brightTexture", resources, iBrightTextureResource) )) {
			return false;
		}

		return true;
	}


	bool AppRenderGraph::addNodes(Repository& repository, std::size_t width, std::size_t height)
	{
		if (!addShadowRenderers(width, height)
			|| !addDeferredRenderers(repository)
			|| !addForwardRenderers()
		) {
			return false;
		}

		auto clearMask = FrameBufferMask::Mask().set(FrameBufferMask::kColor).set(FrameBufferMask::kDepth);
		auto defaultFBClear = std::make_unique<FBClearNode>("defaultFBClear", clearMask);

		// Node used for setting the irradiance and prefilter textures of the renderers
		auto irradianceTexUnitNode = std::make_unique<TextureUnitNode>("irradianceTexUnitNode", DeferredLightRenderer::TexUnits::kIrradianceMap);
		auto prefilterTexUnitNode = std::make_unique<TextureUnitNode>("prefilterTexUnitNode", DeferredLightRenderer::TexUnits::kPrefilterMap);

		// Node used for combining the shadow renderers and the forward and deferred renderers
		auto texUnitNodeShadow = std::make_unique<TextureUnitNode>("texUnitNodeShadow", DeferredLightRenderer::TexUnits::kShadowMap);
		texUnitNodeShadow->addInput( std::make_unique<RNodeInput>("attach", texUnitNodeShadow.get()) );

		// Node used for combining the zBuffer of the deferred and forward renderers
		auto zBufferCopy = std::make_unique<FBCopyNode>("zBufferCopy", FrameBufferMask::Mask().set(FrameBufferMask::kDepth));
		zBufferCopy->setDimensions1(0, 0, width, height).setDimensions2(0, 0, width, height);

		// Nodes used for blurring the bright colors (bloom)
		auto hBlurNode = std::make_unique<GaussianBlurNode>("hBlurNode", repository, width, height, true);
		auto vBlurNode = std::make_unique<GaussianBlurNode>("vBlurNode", repository, width, height, false);
		auto hBlurTexUnitNode = std::make_unique<TextureUnitNode>("hBlurTexUnitNode", GaussianBlurNode::kColorTextureUnit);
		auto vBlurTexUnitNode = std::make_unique<TextureUnitNode>("vBlurTexUnitNode", GaussianBlurNode::kColorTextureUnit);

		// Node used for combining the bloom and color
		auto combine0TexUnitNode = std::make_unique<TextureUnitNode>("combine0TexUnitNode", CombineNode::kColor0);
		auto combine1TexUnitNode = std::make_unique<TextureUnitNode>("combine1TexUnitNode", CombineNode::kColor1);
		auto combineBloomNode = std::make_unique<CombineNode>("combineBloomNode", repository);

		// Node used for drawing 2D renderables
		auto renderer2D = std::make_unique<Renderer2D>("renderer2D");

		// Link the render graph nodes
		auto resources = getNode("resources"),
			endShadow = getNode("endShadow"),
			gBufferRendererParticles = getNode("gBufferRendererParticles"),
			deferredLightRenderer = getNode("deferredLightRenderer"),
			forwardRendererMesh = getNode("forwardRendererMesh");

		return defaultFBClear->findInput("input")->connect( resources->findOutput("defaultFB") )
			&& irradianceTexUnitNode->findInput("input")->connect( resources->findOutput("irradianceTexture") )
			&& prefilterTexUnitNode->findInput("input")->connect( resources->findOutput("prefilterTexture") )
			&& texUnitNodeShadow->findInput("input")->connect( resources->findOutput("shadowTexture") )
			&& texUnitNodeShadow->findInput("attach")->connect( endShadow->findOutput("attach") )
			&& deferredLightRenderer->findInput("irradiance")->connect( irradianceTexUnitNode->findOutput("output") )
			&& deferredLightRenderer->findInput("prefilter")->connect( prefilterTexUnitNode->findOutput("output") )
			&& deferredLightRenderer->findInput("brdf")->connect( resources->findOutput("brdfTexture") )
			&& deferredLightRenderer->findInput("shadow")->connect( texUnitNodeShadow->findOutput("output") )
			&& zBufferCopy->findInput("input1")->connect( deferredLightRenderer->findOutput("target") )
			&& zBufferCopy->findInput("input2")->connect( gBufferRendererParticles->findOutput("target") )
			&& forwardRendererMesh->findInput("target")->connect( zBufferCopy->findOutput("output") )
			&& forwardRendererMesh->findInput("irradiance")->connect( irradianceTexUnitNode->findOutput("output") )
			&& forwardRendererMesh->findInput("prefilter")->connect( prefilterTexUnitNode->findOutput("output") )
			&& forwardRendererMesh->findInput("brdf")->connect( resources->findOutput("brdfTexture") )
			&& forwardRendererMesh->findInput("shadow")->connect( texUnitNodeShadow->findOutput("output") )
			&& forwardRendererMesh->findInput("color")->connect( resources->findOutput("colorTexture") )
			&& forwardRendererMesh->findInput("bright")->connect( resources->findOutput("brightTexture") )
			&& hBlurTexUnitNode->findInput("input")->connect( forwardRendererMesh->findOutput("bright") )
			&& hBlurNode->findInput("input")->connect( hBlurTexUnitNode->findOutput("output") )
			&& vBlurTexUnitNode->findInput("input")->connect( hBlurNode->findOutput("output") )
			&& vBlurNode->findInput("input")->connect( vBlurTexUnitNode->findOutput("output") )
			&& combine0TexUnitNode->findInput("input")->connect( forwardRendererMesh->findOutput("color") )
			&& combine1TexUnitNode->findInput("input")->connect( vBlurNode->findOutput("output") )
			&& combineBloomNode->findInput("target")->connect( defaultFBClear->findOutput("output") )
			&& combineBloomNode->findInput("color0")->connect( combine0TexUnitNode->findOutput("output") )
			&& combineBloomNode->findInput("color1")->connect( combine1TexUnitNode->findOutput("output") )
			&& renderer2D->findInput("target")->connect( combineBloomNode->findOutput("target") )
			&& addNode( std::move(defaultFBClear) )
			&& addNode( std::move(irradianceTexUnitNode) )
			&& addNode( std::move(prefilterTexUnitNode) )
			&& addNode( std::move(texUnitNodeShadow) )
			&& addNode( std::move(zBufferCopy) )
			&& addNode( std::move(hBlurNode) )
			&& addNode( std::move(vBlurNode) )
			&& addNode( std::move(hBlurTexUnitNode) )
			&& addNode( std::move(vBlurTexUnitNode) )
			&& addNode( std::move(combine0TexUnitNode) )
			&& addNode( std::move(combine1TexUnitNode) )
			&& addNode( std::move(combineBloomNode) )
			&& addNode( std::move(renderer2D) );
	}


	bool AppRenderGraph::addDeferredRenderers(Repository& repository)
	{
		// Create the nodes
		auto clearMask = FrameBufferMask::Mask().set(FrameBufferMask::kColor).set(FrameBufferMask::kDepth);
		auto gFBClear = std::make_unique<FBClearNode>("gFBClear", clearMask);
		auto deferredFBClear = std::make_unique<FBClearNode>("deferredFBClear", clearMask);

		auto gBufferRendererTerrain = std::make_unique<RendererTerrain>("gBufferRendererTerrain");
		auto gBufferRendererMesh = std::make_unique<RendererMesh>("gBufferRendererMesh");
		auto gBufferRendererParticles = std::make_unique<RendererParticles>("gBufferRendererParticles");
		gBufferRendererParticles->addOutput( std::make_unique<RNodeOutput>("attach", gBufferRendererParticles.get()) );

		auto deferredLightRenderer = std::make_unique<DeferredLightRenderer>("deferredLightRenderer", repository);
		deferredLightRenderer->addInput( std::make_unique<RNodeInput>("attach", deferredLightRenderer.get()) );

		auto texUnitNodePosition = std::make_unique<TextureUnitNode>("texUnitNodePosition", DeferredLightRenderer::TexUnits::kPosition);
		auto texUnitNodeNormal = std::make_unique<TextureUnitNode>("texUnitNodeNormal", DeferredLightRenderer::TexUnits::kNormal);
		auto texUnitNodeAlbedo = std::make_unique<TextureUnitNode>("texUnitNodeAlbedo", DeferredLightRenderer::TexUnits::kAlbedo);
		auto texUnitNodeMaterial = std::make_unique<TextureUnitNode>("texUnitNodeMaterial", DeferredLightRenderer::TexUnits::kMaterial);
		auto texUnitNodeEmissive = std::make_unique<TextureUnitNode>("texUnitNodeEmissive", DeferredLightRenderer::TexUnits::kEmissive);

		// Add the nodes and their connections
		RenderNode* resources = getNode("resources");

		return gFBClear->findInput("input")->connect( resources->findOutput("gBuffer") )
			&& deferredFBClear->findInput("input")->connect( resources->findOutput("deferredBuffer") )
			&& gBufferRendererTerrain->findInput("target")->connect( gFBClear->findOutput("output") )
			&& gBufferRendererMesh->findInput("target")->connect( gBufferRendererTerrain->findOutput("target") )
			&& gBufferRendererParticles->findInput("target")->connect( gBufferRendererMesh->findOutput("target") )
			&& texUnitNodePosition->findInput("input")->connect( resources->findOutput("positionTexture") )
			&& texUnitNodeNormal->findInput("input")->connect( resources->findOutput("normalTexture") )
			&& texUnitNodeAlbedo->findInput("input")->connect( resources->findOutput("albedoTexture") )
			&& texUnitNodeMaterial->findInput("input")->connect( resources->findOutput("materialTexture") )
			&& texUnitNodeEmissive->findInput("input")->connect( resources->findOutput("emissiveTexture") )
			&& deferredLightRenderer->findInput("attach")->connect( gBufferRendererParticles->findOutput("attach") )
			&& deferredLightRenderer->findInput("target")->connect( deferredFBClear->findOutput("output") )
			&& deferredLightRenderer->findInput("position")->connect( texUnitNodePosition->findOutput("output") )
			&& deferredLightRenderer->findInput("normal")->connect( texUnitNodeNormal->findOutput("output") )
			&& deferredLightRenderer->findInput("albedo")->connect( texUnitNodeAlbedo->findOutput("output") )
			&& deferredLightRenderer->findInput("material")->connect( texUnitNodeMaterial->findOutput("output") )
			&& deferredLightRenderer->findInput("emissive")->connect( texUnitNodeEmissive->findOutput("output") )
			&& addNode( std::move(gFBClear) )
			&& addNode( std::move(deferredFBClear) )
			&& addNode( std::move(gBufferRendererTerrain) )
			&& addNode( std::move(gBufferRendererMesh) )
			&& addNode( std::move(gBufferRendererParticles) )
			&& addNode( std::move(texUnitNodePosition) )
			&& addNode( std::move(texUnitNodeNormal) )
			&& addNode( std::move(texUnitNodeAlbedo) )
			&& addNode( std::move(texUnitNodeMaterial) )
			&& addNode( std::move(texUnitNodeEmissive) )
			&& addNode( std::move(deferredLightRenderer) );
	}


	bool AppRenderGraph::addForwardRenderers()
	{
		auto forwardRenderer = std::make_unique<RendererMesh>("forwardRendererMesh");
		forwardRenderer->addBindable(std::make_shared<SetDepthMask>());

		auto iIrradianceTexBindable = forwardRenderer->addBindable();
		auto iPrefilterTexBindable = forwardRenderer->addBindable();
		auto iBRDFTexBindable = forwardRenderer->addBindable();
		auto iShadowTexBindable = forwardRenderer->addBindable();
		forwardRenderer->addInput( std::make_unique<BindableRNodeInput<Texture>>("irradiance", forwardRenderer.get(), iIrradianceTexBindable) );
		forwardRenderer->addInput( std::make_unique<BindableRNodeInput<Texture>>("prefilter", forwardRenderer.get(), iPrefilterTexBindable) );
		forwardRenderer->addInput( std::make_unique<BindableRNodeInput<Texture>>("brdf", forwardRenderer.get(), iBRDFTexBindable) );
		forwardRenderer->addInput( std::make_unique<BindableRNodeInput<Texture>>("shadow", forwardRenderer.get(), iShadowTexBindable) );

		auto iColorTexBindable = forwardRenderer->addBindable(nullptr, false);
		forwardRenderer->addInput( std::make_unique<BindableRNodeInput<Texture>>("color", forwardRenderer.get(), iColorTexBindable) );
		forwardRenderer->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("color", forwardRenderer.get(), iColorTexBindable) );

		auto iBrightTexBindable = forwardRenderer->addBindable(nullptr, false);
		forwardRenderer->addInput( std::make_unique<BindableRNodeInput<Texture>>("bright", forwardRenderer.get(), iBrightTexBindable) );
		forwardRenderer->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("bright", forwardRenderer.get(), iBrightTexBindable) );

		return addNode(std::move(forwardRenderer));
	}


	bool AppRenderGraph::addShadowRenderers(std::size_t width, std::size_t height)
	{
		// Create the nodes
		auto shadowFBClear = std::make_unique<FBClearNode>("shadowFBClear", FrameBufferMask::Mask().set(FrameBufferMask::kDepth));

		auto shadowRendererTerrain = std::make_unique<RendererTerrain>("shadowRendererTerrain");
		shadowRendererTerrain->addInput( std::make_unique<graphics::RNodeInput>("attach", shadowRendererTerrain.get()) );

		auto shadowRendererMesh = std::make_unique<RendererMesh>("shadowRendererMesh");
		shadowRendererMesh->addOutput( std::make_unique<graphics::RNodeOutput>("attach", shadowRendererMesh.get()) );

		auto startShadowNode = std::make_unique<StartShadowNode>("startShadow");
		auto endShadowNode = std::make_unique<EndShadowNode>("endShadow");
		endShadowNode->setViewportSize(0, 0, width, height);

		// Add the nodes and their connections
		RenderNode* resources = getNode("resources");

		return shadowFBClear->findInput("input")->connect( resources->findOutput("shadowBuffer") )
			&& shadowRendererTerrain->findInput("attach")->connect( startShadowNode->findOutput("attach") )
			&& shadowRendererTerrain->findInput("target")->connect( shadowFBClear->findOutput("output") )
			&& shadowRendererMesh->findInput("target")->connect( shadowRendererTerrain->findOutput("target") )
			&& endShadowNode->findInput("attach")->connect( shadowRendererMesh->findOutput("attach") )
			&& addNode( std::move(shadowFBClear) )
			&& addNode( std::move(startShadowNode) )
			&& addNode( std::move(shadowRendererTerrain) )
			&& addNode( std::move(shadowRendererMesh) )
			&& addNode( std::move(endShadowNode) );
	}

}
