#include "se/utils/Log.h"
#include "se/graphics/FBClearNode.h"
#include "se/graphics/FBCopyNode.h"
#include "se/graphics/TextureUnitNode.h"
#include "se/graphics/2D/Renderer2D.h"
#include "se/graphics/3D/RendererMesh.h"
#include "se/graphics/3D/RendererTerrain.h"
#include "se/graphics/3D/RendererParticles.h"
#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/UniformBlock.h"
#include "se/graphics/core/UniformVariable.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/app/io/MeshLoader.h"
#include "se/app/io/ShaderLoader.h"
#include "se/app/graphics/GaussianBlurNode.h"
#include "se/app/graphics/TextureUtils.h"
#include "se/app/graphics/DeferredAmbientRenderer.h"
#include "se/app/graphics/DeferredLightSubGraph.h"
#include "se/app/graphics/ShadowRenderSubGraph.h"
#include "se/app/graphics/AppRenderGraph.h"

using namespace se::graphics;
using namespace std::string_literals;

namespace se::app {

	class AppRenderGraph::CombineNode : public BindableRenderNode
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
			addBindable(std::make_shared<SetOperation>(Operation::DepthTest, false));
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
			GraphicsOperations::drawIndexedInstanced(
				PrimitiveType::Triangle,
				mPlane->getIBO().getIndexCount(), mPlane->getIBO().getIndexType()
			);
		};
	};


	AppRenderGraph::AppRenderGraph(Repository& repository, std::size_t width, std::size_t height)
	{
		if (!addResources(repository, width, height)) {
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
		auto shadowRenderSubGraph = dynamic_cast<ShadowRenderSubGraph*>(getNode("shadowRenderSubGraph"));
		zBufferCopy->setDimensions1(0, 0, width, height).setDimensions2(0, 0, width, height);
		hBlurNode->setTextureDimensions(width, height);
		vBlurNode->setTextureDimensions(width, height);
		shadowRenderSubGraph->setCameraResolution(width, height);

		auto zTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("zTexture"))->getTBindable();
		auto positionTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("positionTexture"))->getTBindable();
		auto normalTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("normalTexture"))->getTBindable();
		auto albedoTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("albedoTexture"))->getTBindable();
		auto materialTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("materialTexture"))->getTBindable();
		auto emissiveTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("emissiveTexture"))->getTBindable();
		auto depthStencilTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("depthStencilTexture"))->getTBindable();
		auto colorTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("colorTexture"))->getTBindable();
		auto brightTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("brightTexture"))->getTBindable();
		zTexture->setImage(nullptr, TypeId::Float, ColorFormat::Depth, ColorFormat::Depth24, width, height);
		positionTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGB, ColorFormat::RGB16f, width, height);
		normalTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGB, ColorFormat::RGB16f, width, height);
		albedoTexture->setImage(nullptr, TypeId::UnsignedByte, ColorFormat::RGB, ColorFormat::RGB, width, height);
		materialTexture->setImage(nullptr, TypeId::UnsignedByte, ColorFormat::RGB, ColorFormat::RGB, width, height);
		emissiveTexture->setImage(nullptr, TypeId::UnsignedByte, ColorFormat::RGB, ColorFormat::RGB, width, height);
		depthStencilTexture->setImage(nullptr, TypeId::UnsignedInt24_8, ColorFormat::DepthStencil, ColorFormat::Depth24Stencil8, width, height);
		colorTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGBA, ColorFormat::RGBA16f, width, height);
		brightTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGBA, ColorFormat::RGBA16f, width, height);
	}

// Private functions
	bool AppRenderGraph::addResources(Repository& repository, std::size_t width, std::size_t height)
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

		brdfTexture->setTextureUnit(DeferredAmbientRenderer::TexUnits::kBRDFMap);
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
		shadowTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGBA, ColorFormat::RGBA32f, width, height)
			.setWrapping(TextureWrap::ClampToBorder, TextureWrap::ClampToBorder)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		shadowBuffer->attach(*shadowTexture, FrameBufferAttachment::kColor0);
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

		auto depthStencilTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		depthStencilTexture->setImage(nullptr, TypeId::UnsignedInt24_8, ColorFormat::DepthStencil, ColorFormat::Depth24Stencil8, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		deferredBuffer->attach(*depthStencilTexture, FrameBufferAttachment::kDepthStencil);
		auto iDepthStencilTextureResource = resources->addBindable(depthStencilTexture);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("depthStencilTexture", resources, iDepthStencilTextureResource) )) {
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
		if (!addShadowRenderers(repository, width, height)
			|| !addDeferredRenderers(repository, width, height)
			|| !addForwardRenderers()
		) {
			return false;
		}

		auto clearMask = FrameBufferMask::Mask().set(FrameBufferMask::kColor).set(FrameBufferMask::kDepth);
		auto defaultFBClear = std::make_unique<FBClearNode>("defaultFBClear", clearMask);

		// Node used for setting the irradiance and prefilter textures of the renderers
		auto irradianceTexUnitNode = std::make_unique<TextureUnitNode>("irradianceTexUnitNode", DeferredAmbientRenderer::TexUnits::kIrradianceMap);
		auto prefilterTexUnitNode = std::make_unique<TextureUnitNode>("prefilterTexUnitNode", DeferredAmbientRenderer::TexUnits::kPrefilterMap);

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
			shadowRenderSubGraph = getNode("shadowRenderSubGraph"),
			gBufferRendererParticles = getNode("gBufferRendererParticles"),
			deferredAmbientRenderer = getNode("deferredAmbientRenderer"),
			deferredLightSubGraph = getNode("deferredLightSubGraph"),
			forwardRendererMesh = getNode("forwardRendererMesh");

		return defaultFBClear->findInput("input")->connect( resources->findOutput("defaultFB") )
			&& irradianceTexUnitNode->findInput("input")->connect( resources->findOutput("irradianceTexture") )
			&& prefilterTexUnitNode->findInput("input")->connect( resources->findOutput("prefilterTexture") )
			&& shadowRenderSubGraph->findInput("attach3")->connect( gBufferRendererParticles->findOutput("attach") )
			&& deferredAmbientRenderer->findInput("irradiance")->connect( irradianceTexUnitNode->findOutput("output") )
			&& deferredAmbientRenderer->findInput("prefilter")->connect( prefilterTexUnitNode->findOutput("output") )
			&& deferredAmbientRenderer->findInput("brdf")->connect( resources->findOutput("brdfTexture") )
			&& forwardRendererMesh->findInput("target")->connect( deferredLightSubGraph->findOutput("target") )
			&& forwardRendererMesh->findInput("irradiance")->connect( irradianceTexUnitNode->findOutput("output") )
			&& forwardRendererMesh->findInput("prefilter")->connect( prefilterTexUnitNode->findOutput("output") )
			&& forwardRendererMesh->findInput("brdf")->connect( resources->findOutput("brdfTexture") )
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
			&& addNode( std::move(hBlurNode) )
			&& addNode( std::move(vBlurNode) )
			&& addNode( std::move(hBlurTexUnitNode) )
			&& addNode( std::move(vBlurTexUnitNode) )
			&& addNode( std::move(combine0TexUnitNode) )
			&& addNode( std::move(combine1TexUnitNode) )
			&& addNode( std::move(combineBloomNode) )
			&& addNode( std::move(renderer2D) );
	}


	bool AppRenderGraph::addDeferredRenderers(Repository& repository, std::size_t width, std::size_t height)
	{
		// Create the nodes
		auto clearMask = FrameBufferMask::Mask().set(FrameBufferMask::kColor).set(FrameBufferMask::kDepth);
		auto gFBClear = std::make_unique<FBClearNode>("gFBClear", clearMask);
		auto deferredFBClear = std::make_unique<FBClearNode>("deferredFBClear", clearMask);

		auto gBufferRendererTerrain = std::make_unique<RendererTerrain>("gBufferRendererTerrain");
		auto gBufferRendererMesh = std::make_unique<RendererMesh>("gBufferRendererMesh");
		auto gBufferRendererParticles = std::make_unique<RendererParticles>("gBufferRendererParticles");
		gBufferRendererParticles->addOutput( std::make_unique<RNodeOutput>("attach", gBufferRendererParticles.get()) );

		auto texUnitNodeAmbientPosition = std::make_unique<TextureUnitNode>("texUnitNodeAmbientPosition", DeferredAmbientRenderer::TexUnits::kPosition);
		auto texUnitNodeAmbientNormal = std::make_unique<TextureUnitNode>("texUnitNodeAmbientNormal", DeferredAmbientRenderer::TexUnits::kNormal);
		auto texUnitNodeAmbientAlbedo = std::make_unique<TextureUnitNode>("texUnitNodeAmbientAlbedo", DeferredAmbientRenderer::TexUnits::kAlbedo);
		auto texUnitNodeAmbientMaterial = std::make_unique<TextureUnitNode>("texUnitNodeAmbientMaterial", DeferredAmbientRenderer::TexUnits::kMaterial);
		auto texUnitNodeAmbientEmissive = std::make_unique<TextureUnitNode>("texUnitNodeAmbientEmissive", DeferredAmbientRenderer::TexUnits::kEmissive);

		auto deferredAmbientRenderer = std::make_unique<DeferredAmbientRenderer>("deferredAmbientRenderer", repository);
		deferredAmbientRenderer->addInput( std::make_unique<RNodeInput>("attach", deferredAmbientRenderer.get()) );

		auto texUnitNodeLightPosition = std::make_unique<TextureUnitNode>("texUnitNodeLightPosition", DeferredLightSubGraph::TexUnits::kPosition);
		auto texUnitNodeLightNormal = std::make_unique<TextureUnitNode>("texUnitNodeLightNormal", DeferredLightSubGraph::TexUnits::kNormal);
		auto texUnitNodeLightAlbedo = std::make_unique<TextureUnitNode>("texUnitNodeLightAlbedo", DeferredLightSubGraph::TexUnits::kAlbedo);
		auto texUnitNodeLightMaterial = std::make_unique<TextureUnitNode>("texUnitNodeLightMaterial", DeferredLightSubGraph::TexUnits::kMaterial);

		auto zBufferCopy = std::make_unique<FBCopyNode>("zBufferCopy", FrameBufferMask::Mask().set(FrameBufferMask::kDepth));
		zBufferCopy->setDimensions1(0, 0, width, height).setDimensions2(0, 0, width, height);

		auto deferredLightSubGraph = std::make_unique<DeferredLightSubGraph>("deferredLightSubGraph");
		deferredLightSubGraph->addInput( std::make_unique<RNodeInput>("attach1", deferredLightSubGraph.get()) );
		deferredLightSubGraph->addInput( std::make_unique<RNodeInput>("attach2", deferredLightSubGraph.get()) );

		auto lightStencilRenderer = std::make_unique<LightStencilRenderer>("lightStencilRenderer", *deferredLightSubGraph);
		auto lightColorRenderer = std::make_unique<LightColorRenderer>("lightColorRenderer", *deferredLightSubGraph);

		// Add the nodes and their connections
		RenderNode* resources = getNode("resources");

		return gFBClear->findInput("input")->connect( resources->findOutput("gBuffer") )
			&& deferredFBClear->findInput("input")->connect( resources->findOutput("deferredBuffer") )
			&& gBufferRendererTerrain->findInput("target")->connect( gFBClear->findOutput("output") )
			&& gBufferRendererMesh->findInput("target")->connect( gBufferRendererTerrain->findOutput("target") )
			&& gBufferRendererParticles->findInput("target")->connect( gBufferRendererMesh->findOutput("target") )
			&& texUnitNodeAmbientPosition->findInput("input")->connect( resources->findOutput("positionTexture") )
			&& texUnitNodeAmbientNormal->findInput("input")->connect( resources->findOutput("normalTexture") )
			&& texUnitNodeAmbientAlbedo->findInput("input")->connect( resources->findOutput("albedoTexture") )
			&& texUnitNodeAmbientMaterial->findInput("input")->connect( resources->findOutput("materialTexture") )
			&& texUnitNodeAmbientEmissive->findInput("input")->connect( resources->findOutput("emissiveTexture") )
			&& deferredAmbientRenderer->findInput("attach")->connect( gBufferRendererParticles->findOutput("attach") )
			&& deferredAmbientRenderer->findInput("target")->connect( deferredFBClear->findOutput("output") )
			&& deferredAmbientRenderer->findInput("position")->connect( texUnitNodeAmbientPosition->findOutput("output") )
			&& deferredAmbientRenderer->findInput("normal")->connect( texUnitNodeAmbientNormal->findOutput("output") )
			&& deferredAmbientRenderer->findInput("albedo")->connect( texUnitNodeAmbientAlbedo->findOutput("output") )
			&& deferredAmbientRenderer->findInput("material")->connect( texUnitNodeAmbientMaterial->findOutput("output") )
			&& deferredAmbientRenderer->findInput("emissive")->connect( texUnitNodeAmbientEmissive->findOutput("output") )
			&& texUnitNodeLightPosition->findInput("input")->connect( resources->findOutput("positionTexture") )
			&& texUnitNodeLightNormal->findInput("input")->connect( resources->findOutput("normalTexture") )
			&& texUnitNodeLightAlbedo->findInput("input")->connect( resources->findOutput("albedoTexture") )
			&& texUnitNodeLightMaterial->findInput("input")->connect( resources->findOutput("materialTexture") )
			&& zBufferCopy->findInput("input1")->connect( deferredAmbientRenderer->findOutput("target") )
			&& zBufferCopy->findInput("input2")->connect( gBufferRendererParticles->findOutput("target") )
			&& deferredLightSubGraph->findInput("attach1")->connect( lightStencilRenderer->findOutput("attach") )
			&& deferredLightSubGraph->findInput("attach2")->connect( lightColorRenderer->findOutput("attach") )
			&& deferredLightSubGraph->findInput("target")->connect( zBufferCopy->findOutput("output") )
			&& deferredLightSubGraph->findInput("position")->connect( texUnitNodeLightPosition->findOutput("output") )
			&& deferredLightSubGraph->findInput("normal")->connect( texUnitNodeLightNormal->findOutput("output") )
			&& deferredLightSubGraph->findInput("albedo")->connect( texUnitNodeLightAlbedo->findOutput("output") )
			&& deferredLightSubGraph->findInput("material")->connect( texUnitNodeLightMaterial->findOutput("output") )
			&& addNode( std::move(gFBClear) )
			&& addNode( std::move(deferredFBClear) )
			&& addNode( std::move(gBufferRendererTerrain) )
			&& addNode( std::move(gBufferRendererMesh) )
			&& addNode( std::move(gBufferRendererParticles) )
			&& addNode( std::move(texUnitNodeAmbientPosition) )
			&& addNode( std::move(texUnitNodeAmbientNormal) )
			&& addNode( std::move(texUnitNodeAmbientAlbedo) )
			&& addNode( std::move(texUnitNodeAmbientMaterial) )
			&& addNode( std::move(texUnitNodeAmbientEmissive) )
			&& addNode( std::move(deferredAmbientRenderer) )
			&& addNode( std::move(zBufferCopy) )
			&& addNode( std::move(lightStencilRenderer) )
			&& addNode( std::move(lightColorRenderer) )
			&& addNode( std::move(texUnitNodeLightPosition) )
			&& addNode( std::move(texUnitNodeLightNormal) )
			&& addNode( std::move(texUnitNodeLightAlbedo) )
			&& addNode( std::move(texUnitNodeLightMaterial) )
			&& addNode( std::move(deferredLightSubGraph) );
	}


	bool AppRenderGraph::addForwardRenderers()
	{
		auto forwardRenderer = std::make_unique<RendererMesh>("forwardRendererMesh");
		forwardRenderer->addBindable(std::make_shared<SetDepthMask>());

		auto iIrradianceTexBindable = forwardRenderer->addBindable();
		auto iPrefilterTexBindable = forwardRenderer->addBindable();
		auto iBRDFTexBindable = forwardRenderer->addBindable();
		forwardRenderer->addInput( std::make_unique<BindableRNodeInput<Texture>>("irradiance", forwardRenderer.get(), iIrradianceTexBindable) );
		forwardRenderer->addInput( std::make_unique<BindableRNodeInput<Texture>>("prefilter", forwardRenderer.get(), iPrefilterTexBindable) );
		forwardRenderer->addInput( std::make_unique<BindableRNodeInput<Texture>>("brdf", forwardRenderer.get(), iBRDFTexBindable) );

		auto iColorTexBindable = forwardRenderer->addBindable(nullptr, false);
		forwardRenderer->addInput( std::make_unique<BindableRNodeInput<Texture>>("color", forwardRenderer.get(), iColorTexBindable) );
		forwardRenderer->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("color", forwardRenderer.get(), iColorTexBindable) );

		auto iBrightTexBindable = forwardRenderer->addBindable(nullptr, false);
		forwardRenderer->addInput( std::make_unique<BindableRNodeInput<Texture>>("bright", forwardRenderer.get(), iBrightTexBindable) );
		forwardRenderer->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("bright", forwardRenderer.get(), iBrightTexBindable) );

		forwardRenderer->addOutput( std::make_unique<RNodeOutput>("attach", forwardRenderer.get()) );

		return addNode(std::move(forwardRenderer));
	}


	bool AppRenderGraph::addShadowRenderers(Repository& repository, std::size_t width, std::size_t height)
	{
		// Create the nodes
		auto shadowFBClear = std::make_unique<FBClearNode>("shadowFBClear", FrameBufferMask::Mask().set(FrameBufferMask::kColor));
		auto shadowPositionTexUnitNode = std::make_unique<TextureUnitNode>("shadowPositionTexUnitNode", MergeShadowsNode::kPositionTextureUnit);
		auto shadowNormalTexUnitNode = std::make_unique<TextureUnitNode>("shadowNormalTexUnitNode", MergeShadowsNode::kNormalTextureUnit);

		auto shadowRenderSubGraph = std::make_unique<ShadowRenderSubGraph>("shadowRenderSubGraph", repository);
		shadowRenderSubGraph->addInput( std::make_unique<RNodeInput>("attach1", shadowRenderSubGraph.get()) );
		shadowRenderSubGraph->addInput( std::make_unique<RNodeInput>("attach2", shadowRenderSubGraph.get()) );
		shadowRenderSubGraph->addInput( std::make_unique<RNodeInput>("attach3", shadowRenderSubGraph.get()) );
		shadowRenderSubGraph->addOutput( std::make_unique<RNodeOutput>("attach", shadowRenderSubGraph.get()) );
		shadowRenderSubGraph->setCameraResolution(width, height);

		auto shadowRendererTerrain = std::make_unique<ShadowRendererTerrain>("shadowRendererTerrain", *shadowRenderSubGraph);
		auto shadowRendererMesh = std::make_unique<ShadowRendererMesh>("shadowRendererMesh", *shadowRenderSubGraph);
		shadowRenderSubGraph->getShadowUniformsUpdater()->setRenderers({ shadowRendererTerrain.get(), shadowRendererMesh.get() });

		// Add the nodes and their connections
		RenderNode* resources = getNode("resources");

		return shadowFBClear->findInput("input")->connect( resources->findOutput("shadowBuffer") )
			&& shadowPositionTexUnitNode->findInput("input")->connect( resources->findOutput("positionTexture") )
			&& shadowNormalTexUnitNode->findInput("input")->connect( resources->findOutput("normalTexture") )
			&& shadowRenderSubGraph->findInput("target")->connect( shadowFBClear->findOutput("output") )
			&& shadowRenderSubGraph->findInput("position")->connect( shadowPositionTexUnitNode->findOutput("output") )
			&& shadowRenderSubGraph->findInput("normal")->connect( shadowNormalTexUnitNode->findOutput("output") )
			&& shadowRenderSubGraph->findInput("attach1")->connect( shadowRendererTerrain->findOutput("attach") )
			&& shadowRenderSubGraph->findInput("attach2")->connect( shadowRendererMesh->findOutput("attach") )
			&& addNode( std::move(shadowFBClear) )
			&& addNode( std::move(shadowPositionTexUnitNode) )
			&& addNode( std::move(shadowNormalTexUnitNode) )
			&& addNode( std::move(shadowRendererTerrain) )
			&& addNode( std::move(shadowRendererMesh) )
			&& addNode( std::move(shadowRenderSubGraph) );
	}

}
