#include <array>
#include <memory>
#include "se/utils/Log.h"
#include "se/utils/Repository.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/graphics/Pass.h"
#include "se/graphics/Technique.h"
#include "se/graphics/FBClearNode.h"
#include "se/graphics/FBCopyNode.h"
#include "se/graphics/TextureUnitNode.h"
#include "se/graphics/2D/Renderer2D.h"
#include "se/graphics/3D/Renderer3D.h"
#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/UniformBlock.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/app/AppRenderer.h"
#include "se/app/Application.h"
#include "se/app/EntityDatabase.h"
#include "se/app/TransformsComponent.h"
#include "se/app/LightProbe.h"
#include "se/app/graphics/GaussianBlurNode.h"
#include "se/app/graphics/TextureUtils.h"
#include "se/app/loaders/MeshLoader.h"
#include "se/app/loaders/TechniqueLoader.h"
#include "se/app/events/ContainerEvent.h"

using namespace se::graphics;
using namespace std::string_literals;

namespace se::app {

	struct AppRenderer::ShaderLightSource
	{
		glm::vec3 position;
		float padding[1];
		glm::vec3 direction;
		unsigned int type;
		glm::vec4 color;
		float intensity;
		float range;
		float lightAngleScale;
		float lightAngleOffset;
	};


	struct AppRenderer::EnvironmentTexUnits
	{
		static constexpr int kIrradianceMap	= 0;
		static constexpr int kPrefilterMap	= 1;
		static constexpr int kBRDFMap		= 2;
		static constexpr int kShadowMap		= 3;
	};


	struct AppRenderer::DeferredTexUnits
	{
		static constexpr int kPosition		= 4;
		static constexpr int kNormal		= 5;
		static constexpr int kAlbedo		= 6;
		static constexpr int kMaterial		= 7;
		static constexpr int kEmissive		= 8;
	};


	class AppRenderer::ShadowRenderer : public Renderer3D
	{
	private:
		std::size_t mShadowResolution;
		std::size_t mWidth, mHeight;
	public:
		ShadowRenderer(const std::string& name) : Renderer3D(name), mShadowResolution(0), mWidth(0), mHeight(0)
		{
			addBindable(std::make_shared<DepthTestOperation>());
		};

		ShadowRenderer& setShadowResolution(std::size_t shadowResolution)
		{
			mShadowResolution = shadowResolution;
			return *this;
		};

		ShadowRenderer& setViewportResolution(std::size_t width, std::size_t height)
		{
			mWidth = width;
			mHeight = height;
			return *this;
		};

		virtual void execute() override
		{
			GraphicsOperations::setViewport(0, 0, mShadowResolution, mShadowResolution);
			GraphicsOperations::setCullingMode(FaceMode::Front);
			Renderer3D::execute();
			GraphicsOperations::setCullingMode(FaceMode::Back);
			GraphicsOperations::setViewport(0, 0, mWidth, mHeight);
		};
	};


	class AppRenderer::CombineNode : public BindableRenderNode
	{
	public:
		static constexpr int kColor0 = 0;
		static constexpr int kColor1 = 1;
	private:
		std::shared_ptr<RenderableMesh> mPlane;

	public:
		CombineNode(
			const std::string& name, utils::Repository& repository,
			std::shared_ptr<RenderableMesh> plane
		) : BindableRenderNode(name), mPlane(plane)
		{
			auto iTargetBindable = addBindable();
			addInput( std::make_unique<BindableRNodeInput<FrameBuffer>>("target", this, iTargetBindable) );
			addInput( std::make_unique<BindableRNodeInput<Texture>>("color0", this, addBindable()) );
			addInput( std::make_unique<BindableRNodeInput<Texture>>("color1", this, addBindable()) );
			addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("target", this, iTargetBindable) );

			auto program = repository.find<std::string, Program>("fragmentCombineHDR");
			if (!program) {
				program = TechniqueLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentCombineHDR.glsl");
				repository.add("programCombineHDR"s, program);
			}

			addBindable(program);
			addBindable(std::make_shared<UniformVariableValue<glm::mat4>>("uModelMatrix", *program, glm::mat4(1.0f)));
			addBindable(std::make_shared<UniformVariableValue<glm::mat4>>("uViewMatrix", *program, glm::mat4(1.0f)));
			addBindable(std::make_shared<UniformVariableValue<glm::mat4>>("uProjectionMatrix", *program, glm::mat4(1.0f)));
			addBindable(std::make_shared<UniformVariableValue<int>>("uColor0", *program, kColor0));
			addBindable(std::make_shared<UniformVariableValue<int>>("uColor1", *program, kColor1));
		};

		virtual void execute() override
		{
			bind();
			mPlane->draw();
		};
	};


	AppRenderer::AppRenderer(Application& application, const ShadowData& shadowData, std::size_t width, std::size_t height) :
		ISystem(application.getEntityDatabase()), mApplication(application), mShadowData(shadowData),
		mCameraEntity(kNullEntity), mShadowEntity(kNullEntity)
	{
		mApplication.getEventManager().subscribe(this, Topic::Camera);
		mApplication.getEventManager().subscribe(this, Topic::Resize);
		mApplication.getEventManager().subscribe(this, Topic::Shadow);
		mApplication.getEntityDatabase().addSystem(this, EntityDatabase::ComponentMask().set<LightComponent>().set<LightProbe>());

		GraphicsOperations::setViewport(0, 0, width, height);

		// Create the buffer used for storing the light sources data
		mLightsBuffer = std::make_shared<UniformBuffer>();
		utils::FixedVector<ShaderLightSource, kMaxLights> lightsBufferData(kMaxLights);
		mLightsBuffer->resizeAndCopy(lightsBufferData.data(), lightsBufferData.size());

		// Create the plane used for rendering the framebuffers
		RawMesh planeRawMesh;
		planeRawMesh.positions = { {-1.0f,-1.0f, 0.0f }, { 1.0f,-1.0f, 0.0f }, {-1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f } };
		planeRawMesh.faceIndices = { 0, 1, 2, 1, 3, 2, };
		auto planeMesh = std::make_unique<Mesh>(MeshLoader::createGraphicsMesh(planeRawMesh));
		mPlaneRenderable = std::make_shared<RenderableMesh>(std::move(planeMesh));

		// Create the resources of the renderers
		auto& graphicsEngine = *mApplication.getExternalTools().graphicsEngine;
		auto resources = dynamic_cast<BindableRenderNode*>(graphicsEngine.getRenderGraph().getNode("resources"));

		auto shadowBuffer = std::make_shared<FrameBuffer>();
		shadowBuffer->setColorBuffer(false);
		auto iShadowBufferResource = resources->addBindable(shadowBuffer);
		resources->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("shadowBuffer", resources, iShadowBufferResource) );

		auto shadowTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		shadowTexture->setImage(nullptr, TypeId::Float, ColorFormat::Depth, ColorFormat::Depth, mShadowData.resolution, mShadowData.resolution)
			.setWrapping(TextureWrap::ClampToBorder, TextureWrap::ClampToBorder)
			.setBorderColor(1.0f, 1.0f, 1.0f, 1.0f)
			.setFiltering(TextureFilter::Nearest, TextureFilter::Nearest);
		shadowBuffer->attach(*shadowTexture, FrameBufferAttachment::kDepth);

		auto gBuffer = std::make_shared<FrameBuffer>();
		auto iGBufferResource = resources->addBindable(gBuffer);
		resources->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("gBuffer", resources, iGBufferResource) );

		auto zTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		zTexture->setImage(nullptr, TypeId::Float, ColorFormat::Depth, ColorFormat::Depth24, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		gBuffer->attach(*zTexture, FrameBufferAttachment::kDepth);

		auto positionTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		positionTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGB, ColorFormat::RGB16f, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		gBuffer->attach(*positionTexture, FrameBufferAttachment::kColor0);

		auto normalTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		normalTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGB, ColorFormat::RGB16f, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		gBuffer->attach(*normalTexture, FrameBufferAttachment::kColor0 + 1);

		auto albedoTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		albedoTexture->setImage(nullptr, TypeId::UnsignedByte, ColorFormat::RGB, ColorFormat::RGB, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		gBuffer->attach(*albedoTexture, FrameBufferAttachment::kColor0 + 2);

		auto materialTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		materialTexture->setImage(nullptr, TypeId::UnsignedByte, ColorFormat::RGB, ColorFormat::RGB, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		gBuffer->attach(*materialTexture, FrameBufferAttachment::kColor0 + 3);

		auto emissiveTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		emissiveTexture->setImage(nullptr, TypeId::UnsignedByte, ColorFormat::RGB, ColorFormat::RGB, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		gBuffer->attach(*emissiveTexture, FrameBufferAttachment::kColor0 + 4);

		auto deferredBuffer = std::make_shared<FrameBuffer>();
		auto iDeferredBufferResource = resources->addBindable(deferredBuffer);
		resources->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("deferredBuffer", resources, iDeferredBufferResource) );

		auto depthTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		depthTexture->setImage(nullptr, TypeId::Float, ColorFormat::Depth, ColorFormat::Depth24, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		deferredBuffer->attach(*depthTexture, FrameBufferAttachment::kDepth);

		auto colorTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		colorTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGBA, ColorFormat::RGBA16f, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		deferredBuffer->attach(*colorTexture, FrameBufferAttachment::kColor0);

		auto brightTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		brightTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGBA, ColorFormat::RGBA16f, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		deferredBuffer->attach(*brightTexture, FrameBufferAttachment::kColor0 + 1);

		mIrradianceTextureResource = resources->addBindable();
		resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("irradianceTexture", resources, mIrradianceTextureResource) );

		mPrefilterTextureResource = resources->addBindable();
		resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("prefilterTexture", resources, mPrefilterTextureResource) );

		auto brdfTexture = TextureUtils::precomputeBRDF(512);
		brdfTexture->setTextureUnit(EnvironmentTexUnits::kBRDFMap);
		auto iBRDFTextureResource = resources->addBindable(brdfTexture);
		resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("brdfTexture", resources, iBRDFTextureResource) );

		{	// Create the FrameBuffer nodes
			auto clearMask = FrameBufferMask::Mask().set(FrameBufferMask::kColor).set(FrameBufferMask::kDepth);
			graphicsEngine.getRenderGraph().addNode(std::make_unique<FBClearNode>("defaultFBClear", clearMask));
			graphicsEngine.getRenderGraph().addNode(std::make_unique<FBClearNode>("shadowFBClear", FrameBufferMask::Mask().set(FrameBufferMask::kDepth)));
			graphicsEngine.getRenderGraph().addNode(std::make_unique<FBClearNode>("gFBClear", clearMask));
			graphicsEngine.getRenderGraph().addNode(std::make_unique<FBClearNode>("deferredFBClear", clearMask));

			auto zBufferCopy = std::make_unique<FBCopyNode>("zBufferCopy", FrameBufferMask::Mask().set(FrameBufferMask::kDepth));
			zBufferCopy->setDimensions1(0, 0, width, height).setDimensions2(0, 0, width, height);
			graphicsEngine.getRenderGraph().addNode(std::move(zBufferCopy));
		}

		{	// Create the shadowRenderer
			auto shadowRenderer = std::make_unique<ShadowRenderer>("shadowRenderer");
			shadowRenderer->setShadowResolution(mShadowData.resolution)
				.setViewportResolution(width, height);

			auto iTargetBindable = shadowRenderer->addBindable();
			shadowRenderer->addInput( std::make_unique<BindableRNodeInput<FrameBuffer>>("target", shadowRenderer.get(), iTargetBindable) );
			shadowRenderer->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("target", shadowRenderer.get(), iTargetBindable) );

			auto iShadowTexBindable = shadowRenderer->addBindable(shadowTexture, false);
			shadowRenderer->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("shadow", shadowRenderer.get(), iShadowTexBindable) );

			graphicsEngine.getRenderGraph().addNode( std::make_unique<TextureUnitNode>("texUnitNodeShadow", EnvironmentTexUnits::kShadowMap) );
			graphicsEngine.getRenderGraph().addNode(std::move(shadowRenderer));
		}

		{	// Create the gBufferRenderer
			auto gBufferRenderer = std::make_unique<Renderer3D>("gBufferRenderer");

			auto iGBufferBindable = gBufferRenderer->addBindable();
			gBufferRenderer->addInput( std::make_unique<BindableRNodeInput<FrameBuffer>>("gBuffer", gBufferRenderer.get(), iGBufferBindable) );
			gBufferRenderer->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("gBuffer", gBufferRenderer.get(), iGBufferBindable) );

			auto iDepthTexBindable = gBufferRenderer->addBindable(zTexture, false);
			auto iPositionTexBindable = gBufferRenderer->addBindable(positionTexture, false);
			auto iNormalTexBindable = gBufferRenderer->addBindable(normalTexture, false);
			auto iAlbedoTexBindable = gBufferRenderer->addBindable(albedoTexture, false);
			auto iMaterialTexBindable = gBufferRenderer->addBindable(materialTexture, false);
			auto iEmissiveTexBindable = gBufferRenderer->addBindable(emissiveTexture, false);
			gBufferRenderer->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("zBuffer", gBufferRenderer.get(), iDepthTexBindable) );
			gBufferRenderer->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("position", gBufferRenderer.get(), iPositionTexBindable) );
			gBufferRenderer->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("normal", gBufferRenderer.get(), iNormalTexBindable) );
			gBufferRenderer->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("albedo", gBufferRenderer.get(), iAlbedoTexBindable) );
			gBufferRenderer->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("material", gBufferRenderer.get(), iMaterialTexBindable) );
			gBufferRenderer->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("emissive", gBufferRenderer.get(), iEmissiveTexBindable) );

			graphicsEngine.getRenderGraph().addNode( std::make_unique<TextureUnitNode>("texUnitNodePosition", DeferredTexUnits::kPosition) );
			graphicsEngine.getRenderGraph().addNode( std::make_unique<TextureUnitNode>("texUnitNodeNormal", DeferredTexUnits::kNormal) );
			graphicsEngine.getRenderGraph().addNode( std::make_unique<TextureUnitNode>("texUnitNodeAlbedo", DeferredTexUnits::kAlbedo) );
			graphicsEngine.getRenderGraph().addNode( std::make_unique<TextureUnitNode>("texUnitNodeMaterial", DeferredTexUnits::kMaterial) );
			graphicsEngine.getRenderGraph().addNode( std::make_unique<TextureUnitNode>("texUnitNodeEmissive", DeferredTexUnits::kEmissive) );
			graphicsEngine.getRenderGraph().addNode(std::move(gBufferRenderer));
		}

		{	// Create the rendererDeferredLight
			auto rendererDeferredLight = std::make_unique<Renderer3D>("rendererDeferredLight");
			rendererDeferredLight->addBindable(std::make_shared<DepthMaskOperation>(false));

			auto iTargetBindable = rendererDeferredLight->addBindable();
			rendererDeferredLight->addInput( std::make_unique<BindableRNodeInput<FrameBuffer>>("target", rendererDeferredLight.get(), iTargetBindable) );
			rendererDeferredLight->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("target", rendererDeferredLight.get(), iTargetBindable) );

			auto iIrradianceTexBindable = rendererDeferredLight->addBindable();
			auto iPrefilterTexBindable = rendererDeferredLight->addBindable();
			auto iBRDFTexBindable = rendererDeferredLight->addBindable();
			auto iShadowTexBindable = rendererDeferredLight->addBindable();
			auto iPositionTexBindable = rendererDeferredLight->addBindable();
			auto iNormalTexBindable = rendererDeferredLight->addBindable();
			auto iAlbedoTexBindable = rendererDeferredLight->addBindable();
			auto iMaterialTexBindable = rendererDeferredLight->addBindable();
			auto iEmissiveTexBindable = rendererDeferredLight->addBindable();
			rendererDeferredLight->addInput( std::make_unique<BindableRNodeInput<Texture>>("irradiance", rendererDeferredLight.get(), iIrradianceTexBindable) );
			rendererDeferredLight->addInput( std::make_unique<BindableRNodeInput<Texture>>("prefilter", rendererDeferredLight.get(), iPrefilterTexBindable) );
			rendererDeferredLight->addInput( std::make_unique<BindableRNodeInput<Texture>>("brdf", rendererDeferredLight.get(), iBRDFTexBindable) );
			rendererDeferredLight->addInput( std::make_unique<BindableRNodeInput<Texture>>("shadow", rendererDeferredLight.get(), iShadowTexBindable) );
			rendererDeferredLight->addInput( std::make_unique<BindableRNodeInput<Texture>>("position", rendererDeferredLight.get(), iPositionTexBindable) );
			rendererDeferredLight->addInput( std::make_unique<BindableRNodeInput<Texture>>("normal", rendererDeferredLight.get(), iNormalTexBindable) );
			rendererDeferredLight->addInput( std::make_unique<BindableRNodeInput<Texture>>("albedo", rendererDeferredLight.get(), iAlbedoTexBindable) );
			rendererDeferredLight->addInput( std::make_unique<BindableRNodeInput<Texture>>("material", rendererDeferredLight.get(), iMaterialTexBindable) );
			rendererDeferredLight->addInput( std::make_unique<BindableRNodeInput<Texture>>("emissive", rendererDeferredLight.get(), iEmissiveTexBindable) );

			graphicsEngine.getRenderGraph().addNode( std::move(rendererDeferredLight) );
		}

		{	// Create the forwardRenderer
			auto forwardRenderer = std::make_unique<Renderer3D>("forwardRenderer");
			forwardRenderer->addBindable(std::make_shared<DepthMaskOperation>(true));

			auto iTargetBindable = forwardRenderer->addBindable();
			forwardRenderer->addInput( std::make_unique<BindableRNodeInput<FrameBuffer>>("target", forwardRenderer.get(), iTargetBindable) );
			forwardRenderer->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("target", forwardRenderer.get(), iTargetBindable) );

			auto iIrradianceTexBindable = forwardRenderer->addBindable();
			auto iPrefilterTexBindable = forwardRenderer->addBindable();
			auto iBRDFTexBindable = forwardRenderer->addBindable();
			auto iShadowTexBindable = forwardRenderer->addBindable();
			forwardRenderer->addInput( std::make_unique<BindableRNodeInput<Texture>>("irradiance", forwardRenderer.get(), iIrradianceTexBindable) );
			forwardRenderer->addInput( std::make_unique<BindableRNodeInput<Texture>>("prefilter", forwardRenderer.get(), iPrefilterTexBindable) );
			forwardRenderer->addInput( std::make_unique<BindableRNodeInput<Texture>>("brdf", forwardRenderer.get(), iBRDFTexBindable) );
			forwardRenderer->addInput( std::make_unique<BindableRNodeInput<Texture>>("shadow", forwardRenderer.get(), iShadowTexBindable) );

			auto iColorTexBindable = forwardRenderer->addBindable(colorTexture, false);
			forwardRenderer->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("color", forwardRenderer.get(), iColorTexBindable) );

			auto iBrightTexBindable = forwardRenderer->addBindable(brightTexture, false);
			forwardRenderer->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("bright", forwardRenderer.get(), iBrightTexBindable) );

			graphicsEngine.getRenderGraph().addNode(std::move(forwardRenderer));
		}

		{	// Nodes used for blurring the bright colors (bloom)
			graphicsEngine.getRenderGraph().addNode( std::make_unique<GaussianBlurNode>("hBlurNode", mApplication.getRepository(), mPlaneRenderable, width, height, true) );
			graphicsEngine.getRenderGraph().addNode( std::make_unique<GaussianBlurNode>("vBlurNode", mApplication.getRepository(), mPlaneRenderable, width, height, false) );
			graphicsEngine.getRenderGraph().addNode( std::make_unique<TextureUnitNode>("hBlurTexUnitNode", GaussianBlurNode::kColorTextureUnit) );
			graphicsEngine.getRenderGraph().addNode( std::make_unique<TextureUnitNode>("vBlurTexUnitNode", GaussianBlurNode::kColorTextureUnit) );
		}

		{	// Node used for combining the bloom and color
			graphicsEngine.getRenderGraph().addNode( std::make_unique<TextureUnitNode>("combine0TexUnitNode", CombineNode::kColor0) );
			graphicsEngine.getRenderGraph().addNode( std::make_unique<TextureUnitNode>("combine1TexUnitNode", CombineNode::kColor1) );
			graphicsEngine.getRenderGraph().addNode( std::make_unique<CombineNode>("combineBloomNode", mApplication.getRepository(), mPlaneRenderable) );
		}

		{	// Create the renderer2D
			auto renderer2D = std::make_unique<Renderer2D>("renderer2D");
			auto targetIndex = renderer2D->addBindable();
			renderer2D->addInput( std::make_unique<BindableRNodeInput<FrameBuffer>>("target", renderer2D.get(), targetIndex) );
			renderer2D->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("target", renderer2D.get(), targetIndex) );
			graphicsEngine.getRenderGraph().addNode( std::move(renderer2D) );
		}

		{	// Link the render graph nodes
			auto defaultFBClear = graphicsEngine.getRenderGraph().getNode("defaultFBClear"),
				shadowFBClear = graphicsEngine.getRenderGraph().getNode("shadowFBClear"),
				gFBClear = graphicsEngine.getRenderGraph().getNode("gFBClear"),
				deferredFBClear = graphicsEngine.getRenderGraph().getNode("deferredFBClear"),
				zBufferCopy = graphicsEngine.getRenderGraph().getNode("zBufferCopy"),
				shadowRenderer = graphicsEngine.getRenderGraph().getNode("shadowRenderer"),
				texUnitNodeShadow = graphicsEngine.getRenderGraph().getNode("texUnitNodeShadow"),
				gBufferRenderer = graphicsEngine.getRenderGraph().getNode("gBufferRenderer"),
				texUnitNodePosition = graphicsEngine.getRenderGraph().getNode("texUnitNodePosition"),
				texUnitNodeNormal = graphicsEngine.getRenderGraph().getNode("texUnitNodeNormal"),
				texUnitNodeAlbedo = graphicsEngine.getRenderGraph().getNode("texUnitNodeAlbedo"),
				texUnitNodeMaterial = graphicsEngine.getRenderGraph().getNode("texUnitNodeMaterial"),
				texUnitNodeEmissive = graphicsEngine.getRenderGraph().getNode("texUnitNodeEmissive"),
				rendererDeferredLight = graphicsEngine.getRenderGraph().getNode("rendererDeferredLight"),
				forwardRenderer = graphicsEngine.getRenderGraph().getNode("forwardRenderer"),
				hBlurNode = graphicsEngine.getRenderGraph().getNode("hBlurNode"),
				vBlurNode = graphicsEngine.getRenderGraph().getNode("vBlurNode"),
				hBlurTexUnitNode = graphicsEngine.getRenderGraph().getNode("hBlurTexUnitNode"),
				vBlurTexUnitNode = graphicsEngine.getRenderGraph().getNode("vBlurTexUnitNode"),
				combine0TexUnitNode = graphicsEngine.getRenderGraph().getNode("combine0TexUnitNode"),
				combine1TexUnitNode = graphicsEngine.getRenderGraph().getNode("combine1TexUnitNode"),
				combineBloomNode = graphicsEngine.getRenderGraph().getNode("combineBloomNode"),
				renderer2D = graphicsEngine.getRenderGraph().getNode("renderer2D");

			defaultFBClear->findInput("input")->connect( resources->findOutput("defaultFB") );
			shadowFBClear->findInput("input")->connect( resources->findOutput("shadowBuffer") );
			gFBClear->findInput("input")->connect( resources->findOutput("gBuffer") );
			deferredFBClear->findInput("input")->connect( resources->findOutput("deferredBuffer") );
			shadowRenderer->findInput("target")->connect( shadowFBClear->findOutput("output") );
			texUnitNodeShadow->findInput("input")->connect( shadowRenderer->findOutput("shadow") );
			gBufferRenderer->findInput("gBuffer")->connect( gFBClear->findOutput("output") );
			texUnitNodePosition->findInput("input")->connect( gBufferRenderer->findOutput("position") );
			texUnitNodeNormal->findInput("input")->connect( gBufferRenderer->findOutput("normal") );
			texUnitNodeAlbedo->findInput("input")->connect( gBufferRenderer->findOutput("albedo") );
			texUnitNodeMaterial->findInput("input")->connect( gBufferRenderer->findOutput("material") );
			texUnitNodeEmissive->findInput("input")->connect( gBufferRenderer->findOutput("emissive") );
			rendererDeferredLight->findInput("target")->connect( deferredFBClear->findOutput("output") );
			rendererDeferredLight->findInput("irradiance")->connect( resources->findOutput("irradianceTexture") );
			rendererDeferredLight->findInput("prefilter")->connect( resources->findOutput("prefilterTexture") );
			rendererDeferredLight->findInput("brdf")->connect( resources->findOutput("brdfTexture") );
			rendererDeferredLight->findInput("shadow")->connect( texUnitNodeShadow->findOutput("output") );
			rendererDeferredLight->findInput("position")->connect( texUnitNodePosition->findOutput("output") );
			rendererDeferredLight->findInput("normal")->connect( texUnitNodeNormal->findOutput("output") );
			rendererDeferredLight->findInput("albedo")->connect( texUnitNodeAlbedo->findOutput("output") );
			rendererDeferredLight->findInput("material")->connect( texUnitNodeMaterial->findOutput("output") );
			rendererDeferredLight->findInput("emissive")->connect( texUnitNodeEmissive->findOutput("output") );
			zBufferCopy->findInput("input1")->connect( rendererDeferredLight->findOutput("target") );
			zBufferCopy->findInput("input2")->connect( gBufferRenderer->findOutput("gBuffer") );
			forwardRenderer->findInput("target")->connect( zBufferCopy->findOutput("output") );
			forwardRenderer->findInput("irradiance")->connect( resources->findOutput("irradianceTexture") );
			forwardRenderer->findInput("prefilter")->connect( resources->findOutput("prefilterTexture") );
			forwardRenderer->findInput("brdf")->connect( resources->findOutput("brdfTexture") );
			forwardRenderer->findInput("shadow")->connect( texUnitNodeShadow->findOutput("output") );
			hBlurTexUnitNode->findInput("input")->connect( forwardRenderer->findOutput("bright") );
			hBlurNode->findInput("input")->connect( hBlurTexUnitNode->findOutput("output") );
			vBlurTexUnitNode->findInput("input")->connect( hBlurNode->findOutput("output") );
			vBlurNode->findInput("input")->connect( vBlurTexUnitNode->findOutput("output") );
			combine0TexUnitNode->findInput("input")->connect( forwardRenderer->findOutput("color") );
			combine1TexUnitNode->findInput("input")->connect( vBlurNode->findOutput("output") );
			combineBloomNode->findInput("target")->connect( defaultFBClear->findOutput("output") );
			combineBloomNode->findInput("color0")->connect( combine0TexUnitNode->findOutput("output") );
			combineBloomNode->findInput("color1")->connect( combine1TexUnitNode->findOutput("output") );
			renderer2D->findInput("target")->connect( combineBloomNode->findOutput("target") );

			graphicsEngine.getRenderGraph().prepareGraph();
		}

		{	// Create the pass and technique used for the deferred lighting
			auto planeTechnique = mApplication.getRepository().find<std::string, Technique>("techniqueDeferredLighting");
			if (!planeTechnique) {
				planeTechnique = std::make_shared<Technique>();
				mApplication.getRepository().add("techniqueDeferredLighting"s, planeTechnique);

				std::shared_ptr<Program> program = TechniqueLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentDeferredLighting.glsl");
				mApplication.getRepository().add("programDeferredLighting"s, program);

				auto rendererDeferredLight = dynamic_cast<Renderer*>(graphicsEngine.getRenderGraph().getNode("rendererDeferredLight"));
				auto lightingPass = std::make_shared<Pass>(*rendererDeferredLight);
				mApplication.getRepository().add("passDeferredLighting"s, lightingPass);

				mNumLights = std::make_shared<UniformVariableValue<unsigned int>>("uNumLights", *program, 0);
				mShadowLightIndex = std::make_shared<UniformVariableValue<unsigned int>>("uShadowLightIndex", *program, kMaxLights);
				mViewPosition = std::make_shared<UniformVariableValue<glm::vec3>>("uViewPosition", *program, glm::vec3(0.0f));
				mShadowViewMatrix = std::make_shared<UniformVariableValue<glm::mat4>>("uShadowViewMatrix", *program, glm::mat4(1.0f));
				mShadowProjectionMatrix = std::make_shared<UniformVariableValue<glm::mat4>>("uShadowProjectionMatrix", *program, glm::mat4(1.0f));
				lightingPass->addBindable(program)
					.addBindable(std::make_shared<UniformVariableValue<glm::mat4>>("uModelMatrix", *program, glm::mat4(1.0f)))
					.addBindable(std::make_shared<UniformVariableValue<glm::mat4>>("uViewMatrix", *program, glm::mat4(1.0f)))
					.addBindable(std::make_shared<UniformVariableValue<glm::mat4>>("uProjectionMatrix", *program, glm::mat4(1.0f)))
					.addBindable(mShadowViewMatrix)
					.addBindable(mShadowProjectionMatrix)
					.addBindable(mViewPosition)
					.addBindable(std::make_shared<UniformVariableValue<int>>("uIrradianceMap", *program, EnvironmentTexUnits::kIrradianceMap))
					.addBindable(std::make_shared<UniformVariableValue<int>>("uPrefilterMap", *program, EnvironmentTexUnits::kPrefilterMap))
					.addBindable(std::make_shared<UniformVariableValue<int>>("uBRDFMap", *program, EnvironmentTexUnits::kBRDFMap))
					.addBindable(std::make_shared<UniformVariableValue<int>>("uShadowMap", *program, EnvironmentTexUnits::kShadowMap))
					.addBindable(std::make_shared<UniformVariableValue<int>>("uPosition", *program, DeferredTexUnits::kPosition))
					.addBindable(std::make_shared<UniformVariableValue<int>>("uNormal", *program, DeferredTexUnits::kNormal))
					.addBindable(std::make_shared<UniformVariableValue<int>>("uAlbedo", *program, DeferredTexUnits::kAlbedo))
					.addBindable(std::make_shared<UniformVariableValue<int>>("uMaterial", *program, DeferredTexUnits::kMaterial))
					.addBindable(std::make_shared<UniformVariableValue<int>>("uEmissive", *program, DeferredTexUnits::kEmissive))
					.addBindable(brdfTexture)
					.addBindable(mLightsBuffer)
					.addBindable(mNumLights)
					.addBindable(mShadowLightIndex)
					.addBindable(std::make_shared<UniformBlock>("LightsBlock", *program));
				planeTechnique->addPass(lightingPass);

				mPlaneRenderable->addTechnique(std::move(planeTechnique));
				graphicsEngine.addRenderable(mPlaneRenderable.get());
			}
		}
	}


	AppRenderer::~AppRenderer()
	{
		mApplication.getEntityDatabase().removeSystem(this);
		mApplication.getEventManager().unsubscribe(this, Topic::Shadow);
		mApplication.getEventManager().unsubscribe(this, Topic::Resize);
		mApplication.getEventManager().unsubscribe(this, Topic::Camera);
	}


	void AppRenderer::notify(const IEvent& event)
	{
		tryCall(&AppRenderer::onCameraEvent, event);
		tryCall(&AppRenderer::onShadowEvent, event);
		tryCall(&AppRenderer::onResizeEvent, event);
	}


	void AppRenderer::onNewEntity(Entity entity)
	{
		auto [lightProbe] = mEntityDatabase.getComponents<LightProbe>(entity);
		if (lightProbe) {
			auto& graphicsEngine = *mApplication.getExternalTools().graphicsEngine;
			auto resources = dynamic_cast<BindableRenderNode*>(graphicsEngine.getRenderGraph().getNode("resources"));

			if (lightProbe->irradianceMap) {
				lightProbe->irradianceMap->setTextureUnit(EnvironmentTexUnits::kIrradianceMap);
				resources->setBindable(mIrradianceTextureResource, lightProbe->irradianceMap);
			}
			if (lightProbe->prefilterMap) {
				lightProbe->prefilterMap->setTextureUnit(EnvironmentTexUnits::kPrefilterMap);
				resources->setBindable(mPrefilterTextureResource, lightProbe->prefilterMap);
			}

			SOMBRA_INFO_LOG << "Entity " << entity << " with LightProbe " << lightProbe << " added successfully";
		}
	}


	void AppRenderer::onRemoveEntity(Entity entity)
	{
		if (mShadowEntity == entity) {
			mShadowEntity = kNullEntity;
			SOMBRA_INFO_LOG << "Shadow Entity " << entity << " removed successfully";
		}

		auto [lightProbe] = mEntityDatabase.getComponents<LightProbe>(entity);
		if (lightProbe) {
			auto& graphicsEngine = *mApplication.getExternalTools().graphicsEngine;
			auto resources = dynamic_cast<BindableRenderNode*>(graphicsEngine.getRenderGraph().getNode("resources"));

			resources->setBindable(mIrradianceTextureResource, nullptr);
			resources->setBindable(mPrefilterTextureResource, nullptr);

			SOMBRA_INFO_LOG << "Entity " << entity << " with LightProbe " << lightProbe << " removed successfully";
		}
	}


	void AppRenderer::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the LightComponents";

		unsigned int i = 0, iShadowLight = kMaxLights;
		std::array<ShaderLightSource, kMaxLights> uBaseLights;
		mEntityDatabase.iterateComponents<TransformsComponent, LightComponent>(
			[&](Entity entity, TransformsComponent* transforms, LightComponent* light) {
				if (i < kMaxLights) {
					uBaseLights[i].type = static_cast<unsigned int>(light->source->type);
					uBaseLights[i].position = transforms->position;
					uBaseLights[i].direction = glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f) * transforms->orientation);
					uBaseLights[i].color = { light->source->color, 1.0f };
					uBaseLights[i].intensity = light->source->intensity;
					switch (light->source->type) {
						case LightSource::Type::Directional: {
							uBaseLights[i].range = std::numeric_limits<float>::max();
						} break;
						case LightSource::Type::Point: {
							uBaseLights[i].range = light->source->range;
						} break;
						case LightSource::Type::Spot: {
							float cosInner = std::cos(light->source->innerConeAngle);
							float cosOuter = std::cos(light->source->outerConeAngle);
							uBaseLights[i].range = light->source->range;
							uBaseLights[i].lightAngleScale = 1.0f / std::max(0.001f, cosInner - cosOuter);
							uBaseLights[i].lightAngleOffset = -cosOuter * uBaseLights[i].lightAngleScale;
						} break;
					}

					if (mShadowEntity == entity) {
						iShadowLight = i;
					}
					++i;
				}
			}
		);

		mNumLights->setValue(i);
		mShadowLightIndex->setValue(iShadowLight);
		mLightsBuffer->copy(uBaseLights.data(), i);

		auto [cameraTransforms] = mEntityDatabase.getComponents<TransformsComponent>(mCameraEntity);
		if (cameraTransforms) {
			mViewPosition->setValue(cameraTransforms->position);
		}

		auto [shadowTransforms] = mEntityDatabase.getComponents<TransformsComponent>(mShadowEntity);
		if (shadowTransforms) {
			mShadowCamera.setPosition(shadowTransforms->position);
			mShadowCamera.setTarget(shadowTransforms->position + glm::vec3(0.0f, 0.0f, 1.0f) * shadowTransforms->orientation);
			mShadowCamera.setUp({ 0.0f, 1.0f, 0.0f });
		}
		mShadowViewMatrix->setValue(mShadowCamera.getViewMatrix());
		mShadowProjectionMatrix->setValue(mShadowCamera.getProjectionMatrix());

		SOMBRA_INFO_LOG << "Update end";
	}


	void AppRenderer::render()
	{
		SOMBRA_INFO_LOG << "Render start";
		mApplication.getExternalTools().graphicsEngine->render();
		SOMBRA_INFO_LOG << "Render end";
	}

// Private functions
	void AppRenderer::onCameraEvent(const ContainerEvent<Topic::Camera, Entity>& event)
	{
		mCameraEntity = event.getValue();
	}


	void AppRenderer::onShadowEvent(const ContainerEvent<Topic::Shadow, Entity>& event)
	{
		auto [transforms, light] = mEntityDatabase.getComponents<TransformsComponent, LightComponent>(event.getValue());
		if (transforms && light) {
			mShadowEntity = event.getValue();

			mShadowCamera.setPosition(transforms->position);
			mShadowCamera.setTarget(transforms->position + glm::vec3(0.0f, 0.0f, 1.0f) * transforms->orientation);
			mShadowCamera.setUp({ 0.0f, 1.0f, 0.0f });

			if (light->source->type == LightSource::Type::Directional) {
				mShadowCamera.setOrthographicProjectionMatrix(
					-mShadowData.size, mShadowData.size, -mShadowData.size, mShadowData.size,
					mShadowData.zNear, mShadowData.zFar
				);
			}
			else if (light->source->type == LightSource::Type::Spot) {
				mShadowCamera.setPerspectiveProjectionMatrix(
					glm::radians(45.0f), 1.0f,
					mShadowData.zNear, mShadowData.zFar
				);
			}
		}
		else {
			SOMBRA_WARN_LOG << "Couldn't set Entity " << event.getValue() << " as Camera";
		}
	}


	void AppRenderer::onResizeEvent(const ResizeEvent& event)
	{
		auto mWidth = static_cast<std::size_t>(event.getWidth());
		auto mHeight = static_cast<std::size_t>(event.getHeight());
		GraphicsOperations::setViewport(0, 0, mWidth, mHeight);
	}

}
