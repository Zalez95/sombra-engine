#include <array>
#include <memory>
#include "se/utils/Log.h"
#include "se/app/Repository.h"
#include "se/graphics/GraphicsEngine.h"
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
#include "se/graphics/core/GraphicsOperations.h"
#include "se/app/AppRenderer.h"
#include "se/app/Application.h"
#include "se/app/TransformsComponent.h"
#include "se/app/graphics/GaussianBlurNode.h"
#include "se/app/graphics/TextureUtils.h"
#include "se/app/io/MeshLoader.h"
#include "se/app/io/ShaderLoader.h"
#include "se/app/events/ContainerEvent.h"

using namespace se::graphics;
using namespace std::string_literals;

namespace se::app {

	class AppRenderer::StartShadowNode : public ViewportResolutionNode
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


	class AppRenderer::EndShadowNode : public ViewportResolutionNode
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


	class AppRenderer::CombineNode : public BindableRenderNode
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


	AppRenderer::AppRenderer(Application& application, const ShadowData& shadowData, std::size_t width, std::size_t height) :
		ISystem(application.getEntityDatabase()), mApplication(application),
		mDeferredLightRenderer(nullptr), mResources(nullptr), mShadowData(shadowData),
		mShadowEntity(kNullEntity), mLightProbeEntity(kNullEntity)
	{
		mApplication.getEventManager().subscribe(this, Topic::Resize);
		mApplication.getEventManager().subscribe(this, Topic::Shadow);
		mApplication.getEntityDatabase().addSystem(this, EntityDatabase::ComponentMask().set<LightComponent>().set<LightProbe>());

		SOMBRA_INFO_LOG << GraphicsOperations::getGraphicsInfo();
		GraphicsOperations::setViewport(0, 0, width, height);

		if (!addResources(width, height)) {
			throw std::runtime_error("Failed to add resources");
		}
		if (!addNodes(width, height)) {
			throw std::runtime_error("Failed to add nodes");
		}
		mApplication.getExternalTools().graphicsEngine->getRenderGraph().prepareGraph();
	}


	AppRenderer::~AppRenderer()
	{
		mApplication.getExternalTools().graphicsEngine->getRenderGraph().clearNodes();
		mApplication.getRepository().findByName<graphics::Mesh>("plane").setFakeUser(false);
		mApplication.getEntityDatabase().removeSystem(this);
		mApplication.getEventManager().unsubscribe(this, Topic::Shadow);
		mApplication.getEventManager().unsubscribe(this, Topic::Resize);
	}


	void AppRenderer::notify(const IEvent& event)
	{
		tryCall(&AppRenderer::onShadowEvent, event);
		tryCall(&AppRenderer::onResizeEvent, event);
	}


	void AppRenderer::onNewComponent(Entity entity, const EntityDatabase::ComponentMask& mask)
	{
		tryCallC(&AppRenderer::onNewLight, entity, mask);
		tryCallC(&AppRenderer::onNewLightProbe, entity, mask);
	}


	void AppRenderer::onRemoveComponent(Entity entity, const EntityDatabase::ComponentMask& mask)
	{
		tryCallC(&AppRenderer::onRemoveLight, entity, mask);
		tryCallC(&AppRenderer::onRemoveLightProbe, entity, mask);
	}


	void AppRenderer::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the LightComponents";

		// Update light probe
		if (mLightProbeEntity != kNullEntity) {
			auto [lightProbe] = mEntityDatabase.getComponents<LightProbe>(mLightProbeEntity, true);
			if (mResources->getBindable(mIrradianceTextureResource) != lightProbe->irradianceMap.get()) {
				mResources->setBindable(mIrradianceTextureResource, lightProbe->irradianceMap.get());
			}
			if (mResources->getBindable(mPrefilterTextureResource) != lightProbe->prefilterMap.get()) {
				mResources->setBindable(mPrefilterTextureResource, lightProbe->prefilterMap.get());
			}
		}

		// Update light sources and shadows
		unsigned int i = 0, iShadowLight = DeferredLightRenderer::kMaxLights;
		std::array<DeferredLightRenderer::ShaderLightSource, DeferredLightRenderer::kMaxLights> uBaseLights;
		mEntityDatabase.iterateComponents<TransformsComponent, LightComponent>(
			[&](Entity entity, TransformsComponent* transforms, LightComponent* light) {
				if (light->source && (i < DeferredLightRenderer::kMaxLights)) {
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
			},
			true
		);

		mDeferredLightRenderer->setLights(uBaseLights.data(), i);
		mDeferredLightRenderer->setShadowLightIndex(iShadowLight);

		SOMBRA_INFO_LOG << "Update end";
	}


	void AppRenderer::render()
	{
		SOMBRA_INFO_LOG << "Render start";
		mApplication.getExternalTools().graphicsEngine->render();
		SOMBRA_INFO_LOG << "Render end";
	}

// Private functions
	void AppRenderer::onNewLight(Entity entity, LightComponent* light)
	{
		SOMBRA_INFO_LOG << "Entity " << entity << " with LightComponent " << light << " added successfully";
	}


	void AppRenderer::onRemoveLight(Entity entity, LightComponent* light)
	{
		if (mShadowEntity == entity) {
			mShadowEntity = kNullEntity;
			SOMBRA_INFO_LOG << "Active Shadow Camera removed";
		}

		SOMBRA_INFO_LOG << "Entity " << entity << " with LightComponent " << light << " removed successfully";
	}


	void AppRenderer::onNewLightProbe(Entity entity, LightProbe* lightProbe)
	{
		mLightProbeEntity = entity;
		SOMBRA_INFO_LOG << "Entity " << entity << " with LightProbe " << lightProbe << " added successfully";
	}


	void AppRenderer::onRemoveLightProbe(Entity entity, LightProbe* lightProbe)
	{
		if (mLightProbeEntity == entity) {
			mLightProbeEntity = kNullEntity;
			mResources->setBindable(mIrradianceTextureResource, nullptr);
			mResources->setBindable(mPrefilterTextureResource, nullptr);
		}

		SOMBRA_INFO_LOG << "Entity " << entity << " with LightProbe " << lightProbe << " removed successfully";
	}


	bool AppRenderer::addResources(std::size_t width, std::size_t height)
	{
		auto& graphicsEngine = *mApplication.getExternalTools().graphicsEngine;
		mResources = dynamic_cast<BindableRenderNode*>(graphicsEngine.getRenderGraph().getNode("resources"));

		RawMesh planeRawMesh;
		planeRawMesh.positions = { {-1.0f,-1.0f, 0.0f }, { 1.0f,-1.0f, 0.0f }, {-1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f } };
		planeRawMesh.indices = { 0, 1, 2, 1, 3, 2, };
		auto planeMesh = std::make_shared<Mesh>(MeshLoader::createGraphicsMesh(planeRawMesh));
		mApplication.getRepository().insert(std::move(planeMesh), "plane").setFakeUser();

		mIrradianceTextureResource = mResources->addBindable();
		if (!mResources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("irradianceTexture", mResources, mIrradianceTextureResource) )) {
			return false;
		}

		mPrefilterTextureResource = mResources->addBindable();
		if (!mResources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("prefilterTexture", mResources, mPrefilterTextureResource) )) {
			return false;
		}

		auto brdfTexture = TextureUtils::precomputeBRDF(512);
		if (!brdfTexture) {
			return false;
		}

		brdfTexture->setTextureUnit(DeferredLightRenderer::TexUnits::kBRDFMap);
		auto iBRDFTextureResource = mResources->addBindable(brdfTexture);
		if (!mResources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("brdfTexture", mResources, iBRDFTextureResource) )) {
			return false;
		}

		auto shadowBuffer = std::make_shared<FrameBuffer>();
		shadowBuffer->setColorBuffer(false);
		auto iShadowBufferResource = mResources->addBindable(shadowBuffer);
		if (!mResources->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("shadowBuffer", mResources, iShadowBufferResource) )) {
			return false;
		}

		auto shadowTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		shadowTexture->setImage(nullptr, TypeId::Float, ColorFormat::Depth, ColorFormat::Depth, mShadowData.resolution, mShadowData.resolution)
			.setWrapping(TextureWrap::ClampToBorder, TextureWrap::ClampToBorder)
			.setBorderColor(1.0f, 1.0f, 1.0f, 1.0f)
			.setFiltering(TextureFilter::Nearest, TextureFilter::Nearest);
		shadowBuffer->attach(*shadowTexture, FrameBufferAttachment::kDepth);
		auto iShadowTextureResource = mResources->addBindable(shadowTexture);
		if (!mResources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("shadowTexture", mResources, iShadowTextureResource) )) {
			return false;
		}

		auto gBuffer = std::make_shared<FrameBuffer>();
		auto iGBufferResource = mResources->addBindable(gBuffer);
		if (!mResources->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("gBuffer", mResources, iGBufferResource) )) {
			return false;
		}

		auto zTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		zTexture->setImage(nullptr, TypeId::Float, ColorFormat::Depth, ColorFormat::Depth24, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		gBuffer->attach(*zTexture, FrameBufferAttachment::kDepth);
		auto iZTextureResource = mResources->addBindable(zTexture);
		if (!mResources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("zTexture", mResources, iZTextureResource) )) {
			return false;
		}

		auto positionTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		positionTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGB, ColorFormat::RGB16f, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		gBuffer->attach(*positionTexture, FrameBufferAttachment::kColor0);
		auto iPositionTextureResource = mResources->addBindable(positionTexture);
		if (!mResources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("positionTexture", mResources, iPositionTextureResource) )) {
			return false;
		}

		auto normalTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		normalTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGB, ColorFormat::RGB16f, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		gBuffer->attach(*normalTexture, FrameBufferAttachment::kColor0 + 1);
		auto iNormalTextureResource = mResources->addBindable(normalTexture);
		if (!mResources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("normalTexture", mResources, iNormalTextureResource) )) {
			return false;
		}

		auto albedoTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		albedoTexture->setImage(nullptr, TypeId::UnsignedByte, ColorFormat::RGB, ColorFormat::RGB, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		gBuffer->attach(*albedoTexture, FrameBufferAttachment::kColor0 + 2);
		auto iAlbedoTextureResource = mResources->addBindable(albedoTexture);
		if (!mResources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("albedoTexture", mResources, iAlbedoTextureResource) )) {
			return false;
		}

		auto materialTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		materialTexture->setImage(nullptr, TypeId::UnsignedByte, ColorFormat::RGB, ColorFormat::RGB, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		gBuffer->attach(*materialTexture, FrameBufferAttachment::kColor0 + 3);
		auto iMaterialTextureResource = mResources->addBindable(materialTexture);
		if (!mResources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("materialTexture", mResources, iMaterialTextureResource) )) {
			return false;
		}

		auto emissiveTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		emissiveTexture->setImage(nullptr, TypeId::UnsignedByte, ColorFormat::RGB, ColorFormat::RGB, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		gBuffer->attach(*emissiveTexture, FrameBufferAttachment::kColor0 + 4);
		auto iEmissiveTextureResource = mResources->addBindable(emissiveTexture);
		if (!mResources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("emissiveTexture", mResources, iEmissiveTextureResource) )) {
			return false;
		}

		auto deferredBuffer = std::make_shared<FrameBuffer>();
		auto iDeferredBufferResource = mResources->addBindable(deferredBuffer);
		if (!mResources->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("deferredBuffer", mResources, iDeferredBufferResource) )) {
			return false;
		}

		auto depthTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		depthTexture->setImage(nullptr, TypeId::Float, ColorFormat::Depth, ColorFormat::Depth24, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		deferredBuffer->attach(*depthTexture, FrameBufferAttachment::kDepth);
		auto iDepthTextureResource = mResources->addBindable(depthTexture);
		if (!mResources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("depthTexture", mResources, iDepthTextureResource) )) {
			return false;
		}

		auto colorTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		colorTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGBA, ColorFormat::RGBA16f, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		deferredBuffer->attach(*colorTexture, FrameBufferAttachment::kColor0);
		auto iColorTextureResource = mResources->addBindable(colorTexture);
		if (!mResources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("colorTexture", mResources, iColorTextureResource) )) {
			return false;
		}

		auto brightTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		brightTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGBA, ColorFormat::RGBA16f, width, height)
			.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
			.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
		deferredBuffer->attach(*brightTexture, FrameBufferAttachment::kColor0 + 1);
		auto iBrightTextureResource = mResources->addBindable(brightTexture);
		if (!mResources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("brightTexture", mResources, iBrightTextureResource) )) {
			return false;
		}

		return true;
	}


	bool AppRenderer::addNodes(std::size_t width, std::size_t height)
	{
		auto& graphicsEngine = *mApplication.getExternalTools().graphicsEngine;
		auto& renderGraph = graphicsEngine.getRenderGraph();

		if (!addShadowRenderers(renderGraph, width, height)
			|| !addDeferredRenderers(renderGraph)
			|| !addForwardRenderers(renderGraph)
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
		auto hBlurNode = std::make_unique<GaussianBlurNode>("hBlurNode", mApplication.getRepository(), width, height, true);
		auto vBlurNode = std::make_unique<GaussianBlurNode>("vBlurNode", mApplication.getRepository(), width, height, false);
		auto hBlurTexUnitNode = std::make_unique<TextureUnitNode>("hBlurTexUnitNode", GaussianBlurNode::kColorTextureUnit);
		auto vBlurTexUnitNode = std::make_unique<TextureUnitNode>("vBlurTexUnitNode", GaussianBlurNode::kColorTextureUnit);

		// Node used for combining the bloom and color
		auto combine0TexUnitNode = std::make_unique<TextureUnitNode>("combine0TexUnitNode", CombineNode::kColor0);
		auto combine1TexUnitNode = std::make_unique<TextureUnitNode>("combine1TexUnitNode", CombineNode::kColor1);
		auto combineBloomNode = std::make_unique<CombineNode>("combineBloomNode", mApplication.getRepository());

		// Node used for drawing 2D renderables
		auto renderer2D = std::make_unique<Renderer2D>("renderer2D");

		// Link the render graph nodes
		auto endShadow = renderGraph.getNode("endShadow"),
			gBufferRendererParticles = renderGraph.getNode("gBufferRendererParticles"),
			deferredLightRenderer = renderGraph.getNode("deferredLightRenderer"),
			forwardRendererMesh = renderGraph.getNode("forwardRendererMesh");

		return defaultFBClear->findInput("input")->connect( mResources->findOutput("defaultFB") )
			&& irradianceTexUnitNode->findInput("input")->connect( mResources->findOutput("irradianceTexture") )
			&& prefilterTexUnitNode->findInput("input")->connect( mResources->findOutput("prefilterTexture") )
			&& texUnitNodeShadow->findInput("input")->connect( mResources->findOutput("shadowTexture") )
			&& texUnitNodeShadow->findInput("attach")->connect( endShadow->findOutput("attach") )
			&& deferredLightRenderer->findInput("irradiance")->connect( irradianceTexUnitNode->findOutput("output") )
			&& deferredLightRenderer->findInput("prefilter")->connect( prefilterTexUnitNode->findOutput("output") )
			&& deferredLightRenderer->findInput("brdf")->connect( mResources->findOutput("brdfTexture") )
			&& deferredLightRenderer->findInput("shadow")->connect( texUnitNodeShadow->findOutput("output") )
			&& zBufferCopy->findInput("input1")->connect( deferredLightRenderer->findOutput("target") )
			&& zBufferCopy->findInput("input2")->connect( gBufferRendererParticles->findOutput("target") )
			&& forwardRendererMesh->findInput("target")->connect( zBufferCopy->findOutput("output") )
			&& forwardRendererMesh->findInput("irradiance")->connect( irradianceTexUnitNode->findOutput("output") )
			&& forwardRendererMesh->findInput("prefilter")->connect( prefilterTexUnitNode->findOutput("output") )
			&& forwardRendererMesh->findInput("brdf")->connect( mResources->findOutput("brdfTexture") )
			&& forwardRendererMesh->findInput("shadow")->connect( texUnitNodeShadow->findOutput("output") )
			&& forwardRendererMesh->findInput("color")->connect( mResources->findOutput("colorTexture") )
			&& forwardRendererMesh->findInput("bright")->connect( mResources->findOutput("brightTexture") )
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
			&& renderGraph.addNode( std::move(defaultFBClear) )
			&& renderGraph.addNode( std::move(irradianceTexUnitNode) )
			&& renderGraph.addNode( std::move(prefilterTexUnitNode) )
			&& renderGraph.addNode( std::move(texUnitNodeShadow) )
			&& renderGraph.addNode( std::move(zBufferCopy) )
			&& renderGraph.addNode( std::move(hBlurNode) )
			&& renderGraph.addNode( std::move(vBlurNode) )
			&& renderGraph.addNode( std::move(hBlurTexUnitNode) )
			&& renderGraph.addNode( std::move(vBlurTexUnitNode) )
			&& renderGraph.addNode( std::move(combine0TexUnitNode) )
			&& renderGraph.addNode( std::move(combine1TexUnitNode) )
			&& renderGraph.addNode( std::move(combineBloomNode) )
			&& renderGraph.addNode( std::move(renderer2D) );
	}


	bool AppRenderer::addDeferredRenderers(graphics::RenderGraph& renderGraph)
	{
		// Create the nodes
		auto clearMask = FrameBufferMask::Mask().set(FrameBufferMask::kColor).set(FrameBufferMask::kDepth);
		auto gFBClear = std::make_unique<FBClearNode>("gFBClear", clearMask);
		auto deferredFBClear = std::make_unique<FBClearNode>("deferredFBClear", clearMask);

		auto gBufferRendererTerrain = std::make_unique<RendererTerrain>("gBufferRendererTerrain");
		auto gBufferRendererMesh = std::make_unique<RendererMesh>("gBufferRendererMesh");
		auto gBufferRendererParticles = std::make_unique<RendererParticles>("gBufferRendererParticles");
		gBufferRendererParticles->addOutput( std::make_unique<RNodeOutput>("attach", gBufferRendererParticles.get()) );

		auto deferredLightRenderer = std::make_unique<DeferredLightRenderer>("deferredLightRenderer", mApplication.getRepository());
		deferredLightRenderer->addInput( std::make_unique<RNodeInput>("attach", deferredLightRenderer.get()) );
		mDeferredLightRenderer = deferredLightRenderer.get();

		auto texUnitNodePosition = std::make_unique<TextureUnitNode>("texUnitNodePosition", DeferredLightRenderer::TexUnits::kPosition);
		auto texUnitNodeNormal = std::make_unique<TextureUnitNode>("texUnitNodeNormal", DeferredLightRenderer::TexUnits::kNormal);
		auto texUnitNodeAlbedo = std::make_unique<TextureUnitNode>("texUnitNodeAlbedo", DeferredLightRenderer::TexUnits::kAlbedo);
		auto texUnitNodeMaterial = std::make_unique<TextureUnitNode>("texUnitNodeMaterial", DeferredLightRenderer::TexUnits::kMaterial);
		auto texUnitNodeEmissive = std::make_unique<TextureUnitNode>("texUnitNodeEmissive", DeferredLightRenderer::TexUnits::kEmissive);

		// Add the nodes and their connections
		return gFBClear->findInput("input")->connect( mResources->findOutput("gBuffer") )
			&& deferredFBClear->findInput("input")->connect( mResources->findOutput("deferredBuffer") )
			&& gBufferRendererTerrain->findInput("target")->connect( gFBClear->findOutput("output") )
			&& gBufferRendererMesh->findInput("target")->connect( gBufferRendererTerrain->findOutput("target") )
			&& gBufferRendererParticles->findInput("target")->connect( gBufferRendererMesh->findOutput("target") )
			&& texUnitNodePosition->findInput("input")->connect( mResources->findOutput("positionTexture") )
			&& texUnitNodeNormal->findInput("input")->connect( mResources->findOutput("normalTexture") )
			&& texUnitNodeAlbedo->findInput("input")->connect( mResources->findOutput("albedoTexture") )
			&& texUnitNodeMaterial->findInput("input")->connect( mResources->findOutput("materialTexture") )
			&& texUnitNodeEmissive->findInput("input")->connect( mResources->findOutput("emissiveTexture") )
			&& deferredLightRenderer->findInput("attach")->connect( gBufferRendererParticles->findOutput("attach") )
			&& deferredLightRenderer->findInput("target")->connect( deferredFBClear->findOutput("output") )
			&& deferredLightRenderer->findInput("position")->connect( texUnitNodePosition->findOutput("output") )
			&& deferredLightRenderer->findInput("normal")->connect( texUnitNodeNormal->findOutput("output") )
			&& deferredLightRenderer->findInput("albedo")->connect( texUnitNodeAlbedo->findOutput("output") )
			&& deferredLightRenderer->findInput("material")->connect( texUnitNodeMaterial->findOutput("output") )
			&& deferredLightRenderer->findInput("emissive")->connect( texUnitNodeEmissive->findOutput("output") )
			&& renderGraph.addNode( std::move(gFBClear) )
			&& renderGraph.addNode( std::move(deferredFBClear) )
			&& renderGraph.addNode( std::move(gBufferRendererTerrain) )
			&& renderGraph.addNode( std::move(gBufferRendererMesh) )
			&& renderGraph.addNode( std::move(gBufferRendererParticles) )
			&& renderGraph.addNode( std::move(texUnitNodePosition) )
			&& renderGraph.addNode( std::move(texUnitNodeNormal) )
			&& renderGraph.addNode( std::move(texUnitNodeAlbedo) )
			&& renderGraph.addNode( std::move(texUnitNodeMaterial) )
			&& renderGraph.addNode( std::move(texUnitNodeEmissive) )
			&& renderGraph.addNode( std::move(deferredLightRenderer) );
	}


	bool AppRenderer::addForwardRenderers(graphics::RenderGraph& renderGraph)
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

		return renderGraph.addNode(std::move(forwardRenderer));
	}


	bool AppRenderer::addShadowRenderers(graphics::RenderGraph& renderGraph, std::size_t width, std::size_t height)
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
		return shadowFBClear->findInput("input")->connect( mResources->findOutput("shadowBuffer") )
			&& shadowRendererTerrain->findInput("attach")->connect( startShadowNode->findOutput("attach") )
			&& shadowRendererTerrain->findInput("target")->connect( shadowFBClear->findOutput("output") )
			&& shadowRendererMesh->findInput("target")->connect( shadowRendererTerrain->findOutput("target") )
			&& endShadowNode->findInput("attach")->connect( shadowRendererMesh->findOutput("attach") )
			&& renderGraph.addNode( std::move(shadowFBClear) )
			&& renderGraph.addNode( std::move(startShadowNode) )
			&& renderGraph.addNode( std::move(shadowRendererTerrain) )
			&& renderGraph.addNode( std::move(shadowRendererMesh) )
			&& renderGraph.addNode( std::move(endShadowNode) );
	}


	void AppRenderer::onShadowEvent(const ContainerEvent<Topic::Shadow, Entity>& event)
	{
		if (mEntityDatabase.hasComponents<TransformsComponent, LightComponent>(event.getValue())) {
			mShadowEntity = event.getValue();
		}
		else {
			SOMBRA_WARN_LOG << "Couldn't set Entity " << event.getValue() << " as Shadow Entity";
		}
	}


	void AppRenderer::onResizeEvent(const ResizeEvent& event)
	{
		auto width = static_cast<std::size_t>(event.getWidth());
		auto height = static_cast<std::size_t>(event.getHeight());
		GraphicsOperations::setViewport(0, 0, width, height);
	}

}
