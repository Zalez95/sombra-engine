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
#include "se/app/graphics/FXAANode.h"
#include "se/app/graphics/TextureUtils.h"
#include "se/app/graphics/SSAONode.h"
#include "se/app/graphics/DeferredAmbientRenderer.h"
#include "se/app/graphics/DeferredLightSubGraph.h"
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
		auto deferredLightSubGraph = dynamic_cast<DeferredLightSubGraph*>(getNode("deferredLightSubGraph"));
		zBufferCopy->setDimensions1(0, 0, width, height).setDimensions2(0, 0, width, height);
		deferredLightSubGraph->getShadowRenderSubGraph()->setResolution(width, height);

		auto zTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("zTexture"))->getTBindable();
		auto positionTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("positionTexture"))->getTBindable();
		auto normalTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("normalTexture"))->getTBindable();
		auto albedoTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("albedoTexture"))->getTBindable();
		auto materialTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("materialTexture"))->getTBindable();
		auto emissiveTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("emissiveTexture"))->getTBindable();
		auto ssaoTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("ssaoTexture"))->getTBindable();
		auto ssaoHBlurTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("ssaoHBlurTexture"))->getTBindable();
		auto depthStencilTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("depthStencilTexture"))->getTBindable();
		auto colorTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("colorTexture"))->getTBindable();
		auto brightTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("brightTexture"))->getTBindable();
		auto bloomHBlurTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("bloomHBlurTexture"))->getTBindable();
		auto combineTexture = dynamic_cast<BindableRNodeOutput<Texture>*>(resources->findOutput("combineTexture"))->getTBindable();
		zTexture->setImage(nullptr, TypeId::Float, ColorFormat::Depth, ColorFormat::Depth24, width, height);
		positionTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGB, ColorFormat::RGB16f, width, height);
		normalTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGB, ColorFormat::RGB16f, width, height);
		albedoTexture->setImage(nullptr, TypeId::UnsignedByte, ColorFormat::RGB, ColorFormat::RGB, width, height);
		materialTexture->setImage(nullptr, TypeId::UnsignedByte, ColorFormat::RGB, ColorFormat::RGB, width, height);
		emissiveTexture->setImage(nullptr, TypeId::UnsignedByte, ColorFormat::RGB, ColorFormat::RGB, width, height);
		ssaoTexture->setImage(nullptr, TypeId::Float, ColorFormat::R, ColorFormat::R, width, height);
		ssaoHBlurTexture->setImage(nullptr, TypeId::Float, ColorFormat::R, ColorFormat::R, width, height);
		depthStencilTexture->setImage(nullptr, TypeId::UnsignedInt24_8, ColorFormat::DepthStencil, ColorFormat::Depth24Stencil8, width, height);
		colorTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGBA, ColorFormat::RGBA16f, width, height);
		brightTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGBA, ColorFormat::RGBA16f, width, height);
		bloomHBlurTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGBA, ColorFormat::RGBA16f, width, height);
		combineTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGBA, ColorFormat::RGBA16f, width, height);
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

		auto gBuffer = std::make_shared<FrameBuffer>();
		auto iGBufferResource = resources->addBindable(gBuffer);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("gBuffer", resources, iGBufferResource) )) {
			return false;
		}

		auto zTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		zTexture->setImage(nullptr, TypeId::Float, ColorFormat::Depth, ColorFormat::Depth24, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		gBuffer->attach(zTexture, FrameBufferAttachment::kDepth);
		auto iZTextureResource = resources->addBindable(zTexture);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("zTexture", resources, iZTextureResource) )) {
			return false;
		}

		auto positionTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		positionTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGB, ColorFormat::RGB16f, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		gBuffer->attach(positionTexture, FrameBufferAttachment::kColor0);
		auto iPositionTextureResource = resources->addBindable(positionTexture);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("positionTexture", resources, iPositionTextureResource) )) {
			return false;
		}

		auto normalTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		normalTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGB, ColorFormat::RGB16f, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		gBuffer->attach(normalTexture, FrameBufferAttachment::kColor0 + 1);
		auto iNormalTextureResource = resources->addBindable(normalTexture);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("normalTexture", resources, iNormalTextureResource) )) {
			return false;
		}

		auto albedoTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		albedoTexture->setImage(nullptr, TypeId::UnsignedByte, ColorFormat::RGB, ColorFormat::RGB, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		gBuffer->attach(albedoTexture, FrameBufferAttachment::kColor0 + 2);
		auto iAlbedoTextureResource = resources->addBindable(albedoTexture);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("albedoTexture", resources, iAlbedoTextureResource) )) {
			return false;
		}

		auto materialTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		materialTexture->setImage(nullptr, TypeId::UnsignedByte, ColorFormat::RGB, ColorFormat::RGB, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		gBuffer->attach(materialTexture, FrameBufferAttachment::kColor0 + 3);
		auto iMaterialTextureResource = resources->addBindable(materialTexture);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("materialTexture", resources, iMaterialTextureResource) )) {
			return false;
		}

		auto emissiveTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		emissiveTexture->setImage(nullptr, TypeId::UnsignedByte, ColorFormat::RGB, ColorFormat::RGB, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		gBuffer->attach(emissiveTexture, FrameBufferAttachment::kColor0 + 4);
		auto iEmissiveTextureResource = resources->addBindable(emissiveTexture);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("emissiveTexture", resources, iEmissiveTextureResource) )) {
			return false;
		}

		auto ssaoBuffer = std::make_shared<FrameBuffer>();
		auto iSSAOBufferResource = resources->addBindable(ssaoBuffer);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("ssaoBuffer", resources, iSSAOBufferResource) )) {
			return false;
		}

		auto ssaoTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		ssaoTexture->setImage(nullptr, TypeId::Float, ColorFormat::R, ColorFormat::R, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		ssaoBuffer->attach(ssaoTexture, FrameBufferAttachment::kColor0);
		auto iSSAOTextureResource = resources->addBindable(ssaoTexture);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("ssaoTexture", resources, iSSAOTextureResource) )) {
			return false;
		}

		auto ssaoHBlurBuffer = std::make_shared<FrameBuffer>();
		auto iSSAOHBlurBufferResource = resources->addBindable(ssaoHBlurBuffer);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("ssaoHBlurBuffer", resources, iSSAOHBlurBufferResource) )) {
			return false;
		}

		auto ssaoHBlurTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		ssaoHBlurTexture->setImage(nullptr, TypeId::Float, ColorFormat::R, ColorFormat::R, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		ssaoHBlurBuffer->attach(ssaoHBlurTexture, FrameBufferAttachment::kColor0);
		auto iSSAOHBlurTextureResource = resources->addBindable(ssaoHBlurTexture);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("ssaoHBlurTexture", resources, iSSAOHBlurTextureResource) )) {
			return false;
		}

		auto deferredBuffer = std::make_shared<FrameBuffer>();
		auto iDeferredBufferResource = resources->addBindable(deferredBuffer);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("deferredBuffer", resources, iDeferredBufferResource) )) {
			return false;
		}

		auto bloomHBlurBuffer = std::make_shared<FrameBuffer>();
		auto iBloomHBlurBufferResource = resources->addBindable(bloomHBlurBuffer);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("bloomHBlurBuffer", resources, iBloomHBlurBufferResource) )) {
			return false;
		}

		auto bloomVBlurBuffer = std::make_shared<FrameBuffer>();
		auto iBloomVBlurBufferResource = resources->addBindable(bloomVBlurBuffer);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("bloomVBlurBuffer", resources, iBloomVBlurBufferResource) )) {
			return false;
		}

		auto depthStencilTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		depthStencilTexture->setImage(nullptr, TypeId::UnsignedInt24_8, ColorFormat::DepthStencil, ColorFormat::Depth24Stencil8, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		deferredBuffer->attach(depthStencilTexture, FrameBufferAttachment::kDepthStencil);
		auto iDepthStencilTextureResource = resources->addBindable(depthStencilTexture);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("depthStencilTexture", resources, iDepthStencilTextureResource) )) {
			return false;
		}

		auto colorTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		colorTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGBA, ColorFormat::RGBA16f, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		deferredBuffer->attach(colorTexture, FrameBufferAttachment::kColor0);
		auto iColorTextureResource = resources->addBindable(colorTexture);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("colorTexture", resources, iColorTextureResource) )) {
			return false;
		}

		auto brightTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		brightTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGBA, ColorFormat::RGBA16f, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		deferredBuffer->attach(brightTexture, FrameBufferAttachment::kColor0 + 1);
		bloomVBlurBuffer->attach(brightTexture, FrameBufferAttachment::kColor0);
		auto iBrightTextureResource = resources->addBindable(brightTexture);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("brightTexture", resources, iBrightTextureResource) )) {
			return false;
		}

		auto bloomHBlurTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		bloomHBlurTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGBA, ColorFormat::RGBA16f, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		bloomHBlurBuffer->attach(bloomHBlurTexture, FrameBufferAttachment::kColor0);
		auto iBloomHBlurTextureResource = resources->addBindable(bloomHBlurTexture);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("bloomHBlurTexture", resources, iBloomHBlurTextureResource) )) {
			return false;
		}

		auto combineBuffer = std::make_shared<FrameBuffer>();
		auto iCombineBufferResource = resources->addBindable(combineBuffer);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("combineBuffer", resources, iCombineBufferResource) )) {
			return false;
		}

		auto combineTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		combineTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGBA, ColorFormat::RGBA16f, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		combineBuffer->attach(combineTexture, FrameBufferAttachment::kColor0);
		auto iCombineTextureResource = resources->addBindable(combineTexture);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("combineTexture", resources, iCombineTextureResource) )) {
			return false;
		}

		return true;
	}


	bool AppRenderGraph::addNodes(Repository& repository, std::size_t width, std::size_t height)
	{
		if (!addDeferredRenderers(repository, width, height)
			|| !addForwardRenderers()
		) {
			return false;
		}

		auto clearMask = FrameBufferMask::Mask().set(FrameBufferMask::kColor).set(FrameBufferMask::kDepth);
		auto defaultFBClear = std::make_unique<FBClearNode>("defaultFBClear", clearMask);
		auto bloomHBlurFBClear = std::make_unique<FBClearNode>("bloomHBlueFBClear", clearMask);
		auto bloomVBlurFBClear = std::make_unique<FBClearNode>("bloomVBlurFBClear", clearMask);
		bloomVBlurFBClear->addInput( std::make_unique<RNodeInput>("attach", bloomVBlurFBClear.get()) );
		auto combineFBClear = std::make_unique<FBClearNode>("combineFBClear", clearMask);

		// Node used for setting the irradiance and prefilter textures of the renderers
		auto irradianceTexUnitNode = std::make_unique<TextureUnitNode>("irradianceTexUnitNode", DeferredAmbientRenderer::TexUnits::kIrradianceMap);
		auto prefilterTexUnitNode = std::make_unique<TextureUnitNode>("prefilterTexUnitNode", DeferredAmbientRenderer::TexUnits::kPrefilterMap);

		// Nodes used for blurring the bright colors (bloom)
		auto bloomHBlurTexUnitNode = std::make_unique<TextureUnitNode>("bloomHBlurTexUnitNode", GaussianBlurNode::kColorTextureUnit);
		auto bloomHBlurNode = std::make_unique<GaussianBlurNode>("bloomHBlurNode", repository, true);
		bloomHBlurNode->addOutput( std::make_unique<RNodeOutput>("attach", bloomHBlurNode.get()) );

		auto bloomVBlurTexUnitNode = std::make_unique<TextureUnitNode>("bloomVBlurTexUnitNode", GaussianBlurNode::kColorTextureUnit);
		auto bloomVBlurNode = std::make_unique<GaussianBlurNode>("bloomVBlurNode", repository, false);
		bloomVBlurNode->addOutput( std::make_unique<RNodeOutput>("attach", bloomVBlurNode.get()) );

		// Node used for combining the bloom and color
		auto combine0TexUnitNode = std::make_unique<TextureUnitNode>("combine0TexUnitNode", CombineNode::kColor0);
		auto combine1TexUnitNode = std::make_unique<TextureUnitNode>("combine1TexUnitNode", CombineNode::kColor1);
		combine1TexUnitNode->addInput( std::make_unique<RNodeInput>("attach", combine1TexUnitNode.get()) );
		auto combineBloomNode = std::make_unique<CombineNode>("combineBloomNode", repository);
		combineBloomNode->addOutput( std::make_unique<RNodeOutput>("attach", combineBloomNode.get()) );

		// Node used for applying FXAA the 3D scene
		auto fxaaTexUnitNode = std::make_unique<TextureUnitNode>("fxaaTexUnitNode", FXAANode::kColorTextureUnit);
		auto fxaaNode = std::make_unique<FXAANode>("fxaaNode", repository);
		fxaaNode->addInput( std::make_unique<RNodeInput>("attach", fxaaNode.get()) );

		// Node used for drawing 2D renderables
		auto renderer2D = std::make_unique<Renderer2D>("renderer2D");

		// Link the render graph nodes
		auto resources = getNode("resources"),
			deferredAmbientRenderer = getNode("deferredAmbientRenderer"),
			deferredLightSubGraph = getNode("deferredLightSubGraph"),
			forwardRendererMesh = getNode("forwardRendererMesh");

		return defaultFBClear->findInput("target")->connect( resources->findOutput("defaultFB") )
			&& irradianceTexUnitNode->findInput("input")->connect( resources->findOutput("irradianceTexture") )
			&& prefilterTexUnitNode->findInput("input")->connect( resources->findOutput("prefilterTexture") )
			&& deferredAmbientRenderer->findInput("irradiance")->connect( irradianceTexUnitNode->findOutput("output") )
			&& deferredAmbientRenderer->findInput("prefilter")->connect( prefilterTexUnitNode->findOutput("output") )
			&& deferredAmbientRenderer->findInput("brdf")->connect( resources->findOutput("brdfTexture") )
			&& forwardRendererMesh->findInput("target")->connect( deferredLightSubGraph->findOutput("target") )
			&& forwardRendererMesh->findInput("irradiance")->connect( irradianceTexUnitNode->findOutput("output") )
			&& forwardRendererMesh->findInput("prefilter")->connect( prefilterTexUnitNode->findOutput("output") )
			&& forwardRendererMesh->findInput("brdf")->connect( resources->findOutput("brdfTexture") )
			&& forwardRendererMesh->findInput("color")->connect( resources->findOutput("colorTexture") )
			&& forwardRendererMesh->findInput("bright")->connect( resources->findOutput("brightTexture") )
			&& bloomHBlurTexUnitNode->findInput("input")->connect( forwardRendererMesh->findOutput("bright") )
			&& bloomHBlurFBClear->findInput("target")->connect( resources->findOutput("bloomHBlurBuffer") )
			&& bloomHBlurNode->findInput("input")->connect( bloomHBlurTexUnitNode->findOutput("output") )
			&& bloomHBlurNode->findInput("target")->connect( bloomHBlurFBClear->findOutput("target") )
			&& bloomVBlurTexUnitNode->findInput("input")->connect( resources->findOutput("bloomHBlurTexture") )
			&& bloomVBlurFBClear->findInput("attach")->connect( bloomHBlurNode->findOutput("attach") )
			&& bloomVBlurFBClear->findInput("target")->connect( resources->findOutput("bloomVBlurBuffer") )
			&& bloomVBlurNode->findInput("input")->connect( bloomVBlurTexUnitNode->findOutput("output") )
			&& bloomVBlurNode->findInput("target")->connect( bloomVBlurFBClear->findOutput("target") )
			&& combine0TexUnitNode->findInput("input")->connect( forwardRendererMesh->findOutput("color") )
			&& combine1TexUnitNode->findInput("attach")->connect( bloomVBlurNode->findOutput("attach") )
			&& combine1TexUnitNode->findInput("input")->connect( forwardRendererMesh->findOutput("bright") )
			&& combineFBClear->findInput("target")->connect( resources->findOutput("combineBuffer") )
			&& combineBloomNode->findInput("color0")->connect( combine0TexUnitNode->findOutput("output") )
			&& combineBloomNode->findInput("color1")->connect( combine1TexUnitNode->findOutput("output") )
			&& combineBloomNode->findInput("target")->connect( combineFBClear->findOutput("target") )
			&& fxaaTexUnitNode->findInput("input")->connect( resources->findOutput("combineTexture") )
			&& fxaaNode->findInput("attach")->connect( combineBloomNode->findOutput("attach") )
			&& fxaaNode->findInput("input")->connect( fxaaTexUnitNode->findOutput("output") )
			&& fxaaNode->findInput("target")->connect( defaultFBClear->findOutput("target") )
			&& renderer2D->findInput("target")->connect( fxaaNode->findOutput("target") )
			&& addNode( std::move(defaultFBClear) )
			&& addNode( std::move(bloomHBlurFBClear) )
			&& addNode( std::move(bloomVBlurFBClear) )
			&& addNode( std::move(irradianceTexUnitNode) )
			&& addNode( std::move(prefilterTexUnitNode) )
			&& addNode( std::move(bloomHBlurNode) )
			&& addNode( std::move(bloomVBlurNode) )
			&& addNode( std::move(bloomHBlurTexUnitNode) )
			&& addNode( std::move(bloomVBlurTexUnitNode) )
			&& addNode( std::move(combine0TexUnitNode) )
			&& addNode( std::move(combine1TexUnitNode) )
			&& addNode( std::move(combineFBClear) )
			&& addNode( std::move(combineBloomNode) )
			&& addNode( std::move(fxaaTexUnitNode) )
			&& addNode( std::move(fxaaNode) )
			&& addNode( std::move(renderer2D) );
	}


	bool AppRenderGraph::addDeferredRenderers(Repository& repository, std::size_t width, std::size_t height)
	{
		// Create the nodes
		auto colorDepthMask = FrameBufferMask::Mask().set(FrameBufferMask::kColor).set(FrameBufferMask::kDepth);
		auto gFBClear = std::make_unique<FBClearNode>("gFBClear", colorDepthMask);
		auto deferredFBClear = std::make_unique<FBClearNode>("deferredFBClear", colorDepthMask);
		auto ssaoFBClear = std::make_unique<FBClearNode>("ssaoFBClear", colorDepthMask);
		ssaoFBClear->addInput( std::make_unique<RNodeInput>("attach", ssaoFBClear.get()) );
		auto ssaoHBlurFBClear = std::make_unique<FBClearNode>("ssaoHBlurFBClear", colorDepthMask);
		ssaoHBlurFBClear->addInput( std::make_unique<RNodeInput>("attach", ssaoHBlurFBClear.get()) );
		auto ssaoVBlurFBClear = std::make_unique<FBClearNode>("ssaoVBlurFBClear", colorDepthMask);
		ssaoVBlurFBClear->addInput( std::make_unique<RNodeInput>("attach", ssaoVBlurFBClear.get()) );

		auto gBufferRendererTerrain = std::make_unique<RendererTerrain>("gBufferRendererTerrain");
		auto gBufferRendererMesh = std::make_unique<RendererMesh>("gBufferRendererMesh");
		auto gBufferRendererParticles = std::make_unique<RendererParticles>("gBufferRendererParticles");
		gBufferRendererParticles->addOutput( std::make_unique<RNodeOutput>("attach", gBufferRendererParticles.get()) );

		auto texUnitNodeSSAOPosition = std::make_unique<TextureUnitNode>("texUnitNodeSSAOPosition", SSAONode::TexUnits::kPosition);
		auto texUnitNodeSSAONormal = std::make_unique<TextureUnitNode>("texUnitNodeSSAONormal", SSAONode::TexUnits::kNormal);

		auto ssaoNode = std::make_unique<SSAONode>("ssaoNode", repository);
		std::size_t ssaoPositionBindableIndex = dynamic_cast<graphics::BindableRNodeInput<Texture>*>(ssaoNode->findInput("position"))->getBindableIndex();
		std::size_t ssaoNormalBindableIndex = dynamic_cast<graphics::BindableRNodeInput<Texture>*>(ssaoNode->findInput("normal"))->getBindableIndex();
		ssaoNode->addOutput( std::make_unique<RNodeOutput>("attach", ssaoNode.get()) );
		ssaoNode->addOutput( std::make_unique<graphics::BindableRNodeOutput<Texture>>("position", ssaoNode.get(), ssaoPositionBindableIndex) );
		ssaoNode->addOutput( std::make_unique<graphics::BindableRNodeOutput<Texture>>("normal", ssaoNode.get(), ssaoNormalBindableIndex) );

		auto ssaoHBlurTexUnitNode = std::make_unique<TextureUnitNode>("ssaoHBlurTexUnitNode", GaussianBlurNode::kColorTextureUnit);
		auto ssaoHBlurNode = std::make_unique<GaussianBlurNode>("ssaoHBlurNode", repository, true);
		ssaoHBlurNode->addOutput( std::make_unique<RNodeOutput>("attach", ssaoHBlurNode.get()) );

		auto ssaoVBlurTexUnitNode = std::make_unique<TextureUnitNode>("ssaoVBlurTexUnitNode", GaussianBlurNode::kColorTextureUnit);
		auto ssaoVBlurNode = std::make_unique<GaussianBlurNode>("ssaoVBlurNode", repository, false);
		ssaoVBlurNode->addOutput( std::make_unique<RNodeOutput>("attach", ssaoVBlurNode.get()) );

		auto texUnitNodeAmbientPosition = std::make_unique<TextureUnitNode>("texUnitNodeAmbientPosition", DeferredAmbientRenderer::TexUnits::kPosition);
		auto texUnitNodeAmbientNormal = std::make_unique<TextureUnitNode>("texUnitNodeAmbientNormal", DeferredAmbientRenderer::TexUnits::kNormal);
		auto texUnitNodeAmbientAlbedo = std::make_unique<TextureUnitNode>("texUnitNodeAmbientAlbedo", DeferredAmbientRenderer::TexUnits::kAlbedo);
		auto texUnitNodeAmbientMaterial = std::make_unique<TextureUnitNode>("texUnitNodeAmbientMaterial", DeferredAmbientRenderer::TexUnits::kMaterial);
		auto texUnitNodeAmbientEmissive = std::make_unique<TextureUnitNode>("texUnitNodeAmbientEmissive", DeferredAmbientRenderer::TexUnits::kEmissive);
		auto texUnitNodeAmbientSSAO = std::make_unique<TextureUnitNode>("texUnitNodeAmbientSSAO", DeferredAmbientRenderer::TexUnits::kSSAO);
		texUnitNodeAmbientSSAO->addInput( std::make_unique<RNodeInput>("attach", texUnitNodeAmbientSSAO.get()) );

		auto deferredAmbientRenderer = std::make_unique<DeferredAmbientRenderer>("deferredAmbientRenderer", repository);

		auto texUnitNodeLightPosition = std::make_unique<TextureUnitNode>("texUnitNodeLightPosition", DeferredLightSubGraph::TexUnits::kPosition);
		auto texUnitNodeLightNormal = std::make_unique<TextureUnitNode>("texUnitNodeLightNormal", DeferredLightSubGraph::TexUnits::kNormal);
		auto texUnitNodeLightAlbedo = std::make_unique<TextureUnitNode>("texUnitNodeLightAlbedo", DeferredLightSubGraph::TexUnits::kAlbedo);
		auto texUnitNodeLightMaterial = std::make_unique<TextureUnitNode>("texUnitNodeLightMaterial", DeferredLightSubGraph::TexUnits::kMaterial);

		auto zBufferCopy = std::make_unique<FBCopyNode>("zBufferCopy", FrameBufferMask::Mask().set(FrameBufferMask::kDepth));
		zBufferCopy->setDimensions1(0, 0, width, height).setDimensions2(0, 0, width, height);

		auto deferredLightSubGraph = std::make_unique<DeferredLightSubGraph>("deferredLightSubGraph");
		deferredLightSubGraph->getShadowRenderSubGraph()->setResolution(width, height);
		deferredLightSubGraph->addInput( std::make_unique<RNodeInput>("attach1", deferredLightSubGraph.get()) );
		deferredLightSubGraph->addInput( std::make_unique<RNodeInput>("attach2", deferredLightSubGraph.get()) );

		auto shadowTerrainProxyRenderer = std::make_unique<ShadowTerrainProxyRenderer>("shadowTerrainProxyRenderer", *deferredLightSubGraph->getShadowRenderSubGraph());
		shadowTerrainProxyRenderer->addOutput( std::make_unique<RNodeOutput>("attach", shadowTerrainProxyRenderer.get()) );
		auto shadowMeshProxyRenderer = std::make_unique<ShadowMeshProxyRenderer>("shadowMeshProxyRenderer", *deferredLightSubGraph->getShadowRenderSubGraph());
		shadowMeshProxyRenderer->addOutput( std::make_unique<RNodeOutput>("attach", shadowMeshProxyRenderer.get()) );

		// Add the nodes and their connections
		RenderNode* resources = getNode("resources");

		return gFBClear->findInput("target")->connect( resources->findOutput("gBuffer") )
			&& deferredFBClear->findInput("target")->connect( resources->findOutput("deferredBuffer") )
			&& gBufferRendererTerrain->findInput("target")->connect( gFBClear->findOutput("target") )
			&& gBufferRendererMesh->findInput("target")->connect( gBufferRendererTerrain->findOutput("target") )
			&& gBufferRendererParticles->findInput("target")->connect( gBufferRendererMesh->findOutput("target") )
			&& texUnitNodeSSAOPosition->findInput("input")->connect( resources->findOutput("positionTexture") )
			&& texUnitNodeSSAONormal->findInput("input")->connect( resources->findOutput("normalTexture") )
			&& ssaoFBClear->findInput("attach")->connect( gBufferRendererParticles->findOutput("attach") )
			&& ssaoFBClear->findInput("target")->connect( resources->findOutput("ssaoBuffer") )
			&& ssaoNode->findInput("target")->connect( ssaoFBClear->findOutput("target") )
			&& ssaoNode->findInput("position")->connect( texUnitNodeSSAOPosition->findOutput("output") )
			&& ssaoNode->findInput("normal")->connect( texUnitNodeSSAONormal->findOutput("output") )
			&& ssaoHBlurTexUnitNode->findInput("input")->connect( resources->findOutput("ssaoTexture") )
			&& ssaoHBlurFBClear->findInput("attach")->connect( ssaoNode->findOutput("attach") )
			&& ssaoHBlurFBClear->findInput("target")->connect( resources->findOutput("ssaoHBlurBuffer") )
			&& ssaoHBlurNode->findInput("input")->connect( ssaoHBlurTexUnitNode->findOutput("output") )
			&& ssaoHBlurNode->findInput("target")->connect( ssaoHBlurFBClear->findOutput("target") )
			&& ssaoVBlurTexUnitNode->findInput("input")->connect( resources->findOutput("ssaoHBlurTexture") )
			&& ssaoVBlurFBClear->findInput("attach")->connect( ssaoHBlurNode->findOutput("attach") )
			&& ssaoVBlurFBClear->findInput("target")->connect( ssaoNode->findOutput("target") )
			&& ssaoVBlurNode->findInput("input")->connect( ssaoVBlurTexUnitNode->findOutput("output") )
			&& ssaoVBlurNode->findInput("target")->connect( ssaoVBlurFBClear->findOutput("target") )
			&& texUnitNodeAmbientPosition->findInput("input")->connect( ssaoNode->findOutput("position") )
			&& texUnitNodeAmbientNormal->findInput("input")->connect( ssaoNode->findOutput("normal") )
			&& texUnitNodeAmbientAlbedo->findInput("input")->connect( resources->findOutput("albedoTexture") )
			&& texUnitNodeAmbientMaterial->findInput("input")->connect( resources->findOutput("materialTexture") )
			&& texUnitNodeAmbientEmissive->findInput("input")->connect( resources->findOutput("emissiveTexture") )
			&& texUnitNodeAmbientSSAO->findInput("attach")->connect( ssaoVBlurNode->findOutput("attach") )
			&& texUnitNodeAmbientSSAO->findInput("input")->connect( resources->findOutput("ssaoTexture") )
			&& deferredAmbientRenderer->findInput("target")->connect( deferredFBClear->findOutput("target") )
			&& deferredAmbientRenderer->findInput("position")->connect( texUnitNodeAmbientPosition->findOutput("output") )
			&& deferredAmbientRenderer->findInput("normal")->connect( texUnitNodeAmbientNormal->findOutput("output") )
			&& deferredAmbientRenderer->findInput("albedo")->connect( texUnitNodeAmbientAlbedo->findOutput("output") )
			&& deferredAmbientRenderer->findInput("material")->connect( texUnitNodeAmbientMaterial->findOutput("output") )
			&& deferredAmbientRenderer->findInput("emissive")->connect( texUnitNodeAmbientEmissive->findOutput("output") )
			&& deferredAmbientRenderer->findInput("ssao")->connect( texUnitNodeAmbientSSAO->findOutput("output") )
			&& texUnitNodeLightPosition->findInput("input")->connect( resources->findOutput("positionTexture") )
			&& texUnitNodeLightNormal->findInput("input")->connect( resources->findOutput("normalTexture") )
			&& texUnitNodeLightAlbedo->findInput("input")->connect( resources->findOutput("albedoTexture") )
			&& texUnitNodeLightMaterial->findInput("input")->connect( resources->findOutput("materialTexture") )
			&& zBufferCopy->findInput("input1")->connect( deferredAmbientRenderer->findOutput("target") )
			&& zBufferCopy->findInput("input2")->connect( gBufferRendererParticles->findOutput("target") )
			&& deferredLightSubGraph->findInput("attach1")->connect( shadowTerrainProxyRenderer->findOutput("attach") )
			&& deferredLightSubGraph->findInput("attach2")->connect( shadowMeshProxyRenderer->findOutput("attach") )
			&& deferredLightSubGraph->findInput("target")->connect( zBufferCopy->findOutput("output") )
			&& deferredLightSubGraph->findInput("position")->connect( texUnitNodeLightPosition->findOutput("output") )
			&& deferredLightSubGraph->findInput("normal")->connect( texUnitNodeLightNormal->findOutput("output") )
			&& deferredLightSubGraph->findInput("albedo")->connect( texUnitNodeLightAlbedo->findOutput("output") )
			&& deferredLightSubGraph->findInput("material")->connect( texUnitNodeLightMaterial->findOutput("output") )
			&& addNode( std::move(gFBClear) )
			&& addNode( std::move(ssaoFBClear) )
			&& addNode( std::move(ssaoHBlurFBClear) )
			&& addNode( std::move(ssaoVBlurFBClear) )
			&& addNode( std::move(deferredFBClear) )
			&& addNode( std::move(gBufferRendererTerrain) )
			&& addNode( std::move(gBufferRendererMesh) )
			&& addNode( std::move(gBufferRendererParticles) )
			&& addNode( std::move(texUnitNodeSSAOPosition) )
			&& addNode( std::move(texUnitNodeSSAONormal) )
			&& addNode( std::move(ssaoNode) )
			&& addNode( std::move(ssaoHBlurNode) )
			&& addNode( std::move(ssaoVBlurNode) )
			&& addNode( std::move(ssaoHBlurTexUnitNode) )
			&& addNode( std::move(ssaoVBlurTexUnitNode) )
			&& addNode( std::move(texUnitNodeAmbientPosition) )
			&& addNode( std::move(texUnitNodeAmbientNormal) )
			&& addNode( std::move(texUnitNodeAmbientAlbedo) )
			&& addNode( std::move(texUnitNodeAmbientMaterial) )
			&& addNode( std::move(texUnitNodeAmbientEmissive) )
			&& addNode( std::move(texUnitNodeAmbientSSAO) )
			&& addNode( std::move(deferredAmbientRenderer) )
			&& addNode( std::move(zBufferCopy) )
			&& addNode( std::move(shadowTerrainProxyRenderer) )
			&& addNode( std::move(shadowMeshProxyRenderer) )
			&& addNode( std::move(texUnitNodeLightPosition) )
			&& addNode( std::move(texUnitNodeLightNormal) )
			&& addNode( std::move(texUnitNodeLightAlbedo) )
			&& addNode( std::move(texUnitNodeLightMaterial) )
			&& addNode( std::move(deferredLightSubGraph) );
	}


	bool AppRenderGraph::addForwardRenderers()
	{
		auto forwardRenderer = std::make_unique<RendererMesh>("forwardRendererMesh");

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

}
