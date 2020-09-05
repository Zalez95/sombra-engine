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
#include "se/app/graphics/LightComponent.h"
#include "se/app/graphics/LightProbe.h"
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


	AppRenderer::AppRenderer(Application& application, std::size_t width, std::size_t height) :
		ISystem(application.getEntityDatabase()), mApplication(application), mCameraEntity(kNullEntity), mCameraUpdated(false)
	{
		mApplication.getEventManager().subscribe(this, Topic::Camera);
		mApplication.getEventManager().subscribe(this, Topic::Resize);
		mApplication.getEntityDatabase().addSystem(this, EntityDatabase::ComponentMask().set<LightProbe>());

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

		// Create the framebuffers of the renderers
		auto& graphicsEngine = *mApplication.getExternalTools().graphicsEngine;
		auto resources = dynamic_cast<BindableRenderNode*>(graphicsEngine.getRenderGraph().getNode("resources"));

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

		{	// Create the FrameBuffer nodes
			auto clearMask = FrameBufferMask::Mask().set(FrameBufferMask::kColor).set(FrameBufferMask::kDepth);
			graphicsEngine.getRenderGraph().addNode(std::make_unique<FBClearNode>("defaultFBClear", clearMask));
			graphicsEngine.getRenderGraph().addNode(std::make_unique<FBClearNode>("gFBClear", clearMask));
			graphicsEngine.getRenderGraph().addNode(std::make_unique<FBClearNode>("deferredFBClear", clearMask));

			auto zBufferCopy = std::make_unique<FBCopyNode>("zBufferCopy", FrameBufferMask::Mask().set(FrameBufferMask::kDepth));
			zBufferCopy->setDimensions1(0, 0, width, height).setDimensions2(0, 0, width, height);
			graphicsEngine.getRenderGraph().addNode(std::move(zBufferCopy));
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

			graphicsEngine.getRenderGraph().addNode(std::move(gBufferRenderer));
		}

		{	// Create the rendererDeferredLight
			auto rendererDeferredLight = std::make_unique<Renderer3D>("rendererDeferredLight");
			rendererDeferredLight->addBindable(std::make_shared<DepthMaskOperation>(false));

			auto iTargetBindable = rendererDeferredLight->addBindable();
			rendererDeferredLight->addInput( std::make_unique<BindableRNodeInput<FrameBuffer>>("target", rendererDeferredLight.get(), iTargetBindable) );
			rendererDeferredLight->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("target", rendererDeferredLight.get(), iTargetBindable) );

			auto iPositionTexBindable = rendererDeferredLight->addBindable();
			auto iNormalTexBindable = rendererDeferredLight->addBindable();
			auto iAlbedoTexBindable = rendererDeferredLight->addBindable();
			auto iMaterialTexBindable = rendererDeferredLight->addBindable();
			auto iEmissiveTexBindable = rendererDeferredLight->addBindable();
			rendererDeferredLight->addInput( std::make_unique<BindableRNodeInput<Texture>>("position", rendererDeferredLight.get(), iPositionTexBindable) );
			rendererDeferredLight->addInput( std::make_unique<BindableRNodeInput<Texture>>("normal", rendererDeferredLight.get(), iNormalTexBindable) );
			rendererDeferredLight->addInput( std::make_unique<BindableRNodeInput<Texture>>("albedo", rendererDeferredLight.get(), iAlbedoTexBindable) );
			rendererDeferredLight->addInput( std::make_unique<BindableRNodeInput<Texture>>("material", rendererDeferredLight.get(), iMaterialTexBindable) );
			rendererDeferredLight->addInput( std::make_unique<BindableRNodeInput<Texture>>("emissive", rendererDeferredLight.get(), iEmissiveTexBindable) );

			graphicsEngine.getRenderGraph().addNode( std::make_unique<TextureUnitNode>("defPositionTexUnitNode", kPosition) );
			graphicsEngine.getRenderGraph().addNode( std::make_unique<TextureUnitNode>("defNormalTexUnitNode", kNormal) );
			graphicsEngine.getRenderGraph().addNode( std::make_unique<TextureUnitNode>("defAlbedoTexUnitNode", kAlbedo) );
			graphicsEngine.getRenderGraph().addNode( std::make_unique<TextureUnitNode>("defMaterialTexUnitNode", kMaterial) );
			graphicsEngine.getRenderGraph().addNode( std::make_unique<TextureUnitNode>("defEmissiveTexUnitNode", kEmissive) );
			graphicsEngine.getRenderGraph().addNode( std::move(rendererDeferredLight) );
		}

		{	// Create the forwardRenderer
			auto forwardRenderer = std::make_unique<Renderer3D>("forwardRenderer");
			forwardRenderer->addBindable(std::make_shared<DepthMaskOperation>(true));

			auto iTargetBindable = forwardRenderer->addBindable();
			forwardRenderer->addInput( std::make_unique<BindableRNodeInput<FrameBuffer>>("target", forwardRenderer.get(), iTargetBindable) );
			forwardRenderer->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("target", forwardRenderer.get(), iTargetBindable) );

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
			static constexpr int kColor0 = 0;
			static constexpr int kColor1 = 1;

			class CombineNode : public BindableRenderNode
			{
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

					auto program = repository.find<std::string, graphics::Program>("fragmentCombineHDR");
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
					mPlane->bind();
					mPlane->draw();
				};
			};

			graphicsEngine.getRenderGraph().addNode( std::make_unique<TextureUnitNode>("combine0TexUnitNode", kColor0) );
			graphicsEngine.getRenderGraph().addNode( std::make_unique<TextureUnitNode>("combine1TexUnitNode", kColor1) );
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
				gFBClear = graphicsEngine.getRenderGraph().getNode("gFBClear"),
				deferredFBClear = graphicsEngine.getRenderGraph().getNode("deferredFBClear"),
				zBufferCopy = graphicsEngine.getRenderGraph().getNode("zBufferCopy"),
				gBufferRenderer = graphicsEngine.getRenderGraph().getNode("gBufferRenderer"),
				defPositionTexUnitNode = graphicsEngine.getRenderGraph().getNode("defPositionTexUnitNode"),
				defNormalTexUnitNode = graphicsEngine.getRenderGraph().getNode("defNormalTexUnitNode"),
				defAlbedoTexUnitNode = graphicsEngine.getRenderGraph().getNode("defAlbedoTexUnitNode"),
				defMaterialTexUnitNode = graphicsEngine.getRenderGraph().getNode("defMaterialTexUnitNode"),
				defEmissiveTexUnitNode = graphicsEngine.getRenderGraph().getNode("defEmissiveTexUnitNode"),
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
			gFBClear->findInput("input")->connect( resources->findOutput("gBuffer") );
			deferredFBClear->findInput("input")->connect( resources->findOutput("deferredBuffer") );
			gBufferRenderer->findInput("gBuffer")->connect( gFBClear->findOutput("output") );
			defPositionTexUnitNode->findInput("input")->connect( gBufferRenderer->findOutput("position") );
			defNormalTexUnitNode->findInput("input")->connect( gBufferRenderer->findOutput("normal") );
			defAlbedoTexUnitNode->findInput("input")->connect( gBufferRenderer->findOutput("albedo") );
			defMaterialTexUnitNode->findInput("input")->connect( gBufferRenderer->findOutput("material") );
			defEmissiveTexUnitNode->findInput("input")->connect( gBufferRenderer->findOutput("emissive") );
			rendererDeferredLight->findInput("target")->connect( deferredFBClear->findOutput("output") );
			rendererDeferredLight->findInput("position")->connect( defPositionTexUnitNode->findOutput("output") );
			rendererDeferredLight->findInput("normal")->connect( defNormalTexUnitNode->findOutput("output") );
			rendererDeferredLight->findInput("albedo")->connect( defAlbedoTexUnitNode->findOutput("output") );
			rendererDeferredLight->findInput("material")->connect( defMaterialTexUnitNode->findOutput("output") );
			rendererDeferredLight->findInput("emissive")->connect( defEmissiveTexUnitNode->findOutput("output") );
			zBufferCopy->findInput("input1")->connect( rendererDeferredLight->findOutput("target") );
			zBufferCopy->findInput("input2")->connect( gBufferRenderer->findOutput("gBuffer") );
			forwardRenderer->findInput("target")->connect( zBufferCopy->findOutput("output") );
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
			auto planeTechnique = mApplication.getRepository().find<std::string, graphics::Technique>("techniqueDeferredLighting");
			if (!planeTechnique) {
				planeTechnique = std::make_shared<Technique>();
				mApplication.getRepository().add("techniqueDeferredLighting"s, planeTechnique);

				std::shared_ptr<graphics::Program> program = TechniqueLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentDeferredLighting.glsl");
				mApplication.getRepository().add("programDeferredLighting"s, program);

				auto rendererDeferredLight = dynamic_cast<Renderer3D*>(graphicsEngine.getRenderGraph().getNode("rendererDeferredLight"));
				auto lightingPass = std::make_shared<Pass>(*rendererDeferredLight);
				mApplication.getRepository().add("passDeferredLighting"s, lightingPass);

				auto brdfTexture = TextureUtils::precomputeBRDF(512);
				brdfTexture->setTextureUnit(kBRDFMap);
				mViewPosition = std::make_shared<UniformVariableValue<glm::vec3>>("uViewPosition", *program, glm::vec3(0.0f));
				mNumLights = std::make_shared<UniformVariableValue<unsigned int>>("uNumLights", *program, 0);
				lightingPass->addBindable(program)
					.addBindable(std::make_shared<UniformVariableValue<glm::mat4>>("uModelMatrix", *program, glm::mat4(1.0f)))
					.addBindable(std::make_shared<UniformVariableValue<glm::mat4>>("uViewMatrix", *program, glm::mat4(1.0f)))
					.addBindable(std::make_shared<UniformVariableValue<glm::mat4>>("uProjectionMatrix", *program, glm::mat4(1.0f)))
					.addBindable(mViewPosition)
					.addBindable(std::make_shared<UniformVariableValue<int>>("uPosition", *program, kPosition))
					.addBindable(std::make_shared<UniformVariableValue<int>>("uNormal", *program, kNormal))
					.addBindable(std::make_shared<UniformVariableValue<int>>("uAlbedo", *program, kAlbedo))
					.addBindable(std::make_shared<UniformVariableValue<int>>("uMaterial", *program, kMaterial))
					.addBindable(std::make_shared<UniformVariableValue<int>>("uEmissive", *program, kEmissive))
					.addBindable(std::make_shared<UniformVariableValue<int>>("uIrradianceMap", *program, kIrradianceMap))
					.addBindable(std::make_shared<UniformVariableValue<int>>("uPrefilterMap", *program, kPrefilterMap))
					.addBindable(std::make_shared<UniformVariableValue<int>>("uBRDFMap", *program, kBRDFMap))
					.addBindable(brdfTexture)
					.addBindable(mLightsBuffer)
					.addBindable(mNumLights)
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
		mApplication.getEventManager().unsubscribe(this, Topic::Resize);
		mApplication.getEventManager().unsubscribe(this, Topic::Camera);
	}


	void AppRenderer::notify(const IEvent& event)
	{
		tryCall(&AppRenderer::onCameraEvent, event);
		tryCall(&AppRenderer::onResizeEvent, event);
	}


	void AppRenderer::onNewEntity(Entity entity)
	{
		if (auto lightProbe = std::get<0>(mEntityDatabase.getComponents<LightProbe>(entity))) {
			auto lightingPass = mApplication.getRepository().find<std::string, graphics::Pass>("passDeferredLighting");
			if (lightProbe->irradianceMap) {
				mIrradianceMap = lightProbe->irradianceMap;
				mIrradianceMap->setTextureUnit(kIrradianceMap);
				lightingPass->addBindable(mIrradianceMap);
			}
			if (lightProbe->prefilterMap) {
				mPrefilterMap = lightProbe->prefilterMap;
				mPrefilterMap->setTextureUnit(kPrefilterMap);
				lightingPass->addBindable(mPrefilterMap);
			}

			SOMBRA_INFO_LOG << "Entity " << entity << " with LightProbe " << lightProbe << " added successfully";
		}
		else {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as LightProbe";
		}
	}


	void AppRenderer::onRemoveEntity(Entity entity)
	{
		auto lightingPass = mApplication.getRepository().find<std::string, graphics::Pass>("passDeferredLighting");
		if (mPrefilterMap) {
			lightingPass->removeBindable(mPrefilterMap);
			mPrefilterMap = nullptr;
		}
		if (mIrradianceMap) {
			lightingPass->removeBindable(mIrradianceMap);
			mIrradianceMap = nullptr;
		}

		SOMBRA_INFO_LOG << "Entity " << entity << " removed successfully";
	}


	void AppRenderer::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the LightComponents";

		unsigned int i = 0;
		std::array<ShaderLightSource, kMaxLights> uBaseLights;
		mEntityDatabase.iterateComponents<TransformsComponent, LightComponent>(
			[&](Entity, TransformsComponent* transforms, LightComponent* light) {
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
					++i;
				}
			}
		);

		mNumLights->setValue(i);
		mLightsBuffer->copy(uBaseLights.data(), i);

		auto [transforms] = mEntityDatabase.getComponents<TransformsComponent>(mCameraEntity);
		if (transforms && (transforms->updated.any() || mCameraUpdated)) {
			mViewPosition->setValue(transforms->position);
		}
		mCameraUpdated = false;

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
		mCameraUpdated = true;
	}


	void AppRenderer::onResizeEvent(const ResizeEvent& event)
	{
		auto mWidth = static_cast<std::size_t>(event.getWidth());
		auto mHeight = static_cast<std::size_t>(event.getHeight());
		graphics::GraphicsOperations::setViewport(0, 0, mWidth, mHeight);
	}

}
