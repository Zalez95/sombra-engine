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
#include "se/app/Application.h"
#include "se/app/AppRenderer.h"
#include "se/app/EntityDatabase.h"
#include "se/app/TransformsComponent.h"
#include "se/app/graphics/Camera.h"
#include "se/app/graphics/LightSource.h"
#include "se/app/graphics/GaussianBlurNode.h"
#include "se/app/loaders/MeshLoader.h"
#include "se/app/loaders/TechniqueLoader.h"

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


	AppRenderer::AppRenderer(
		Application& application, GraphicsEngine& graphicsEngine, CameraSystem& cameraSystem,
		std::size_t width, std::size_t height
	) : ISystem(application.getEntityDatabase()), mGraphicsEngine(graphicsEngine), mCameraSystem(cameraSystem)
	{
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

		auto resources = dynamic_cast<BindableRenderNode*>(mGraphicsEngine.getRenderGraph().getNode("resources"));

		{	// Create the FBClearNodes
			auto clearMask = FrameBufferMask::Mask().set(FrameBufferMask::kColor).set(FrameBufferMask::kDepth);
			mGraphicsEngine.getRenderGraph().addNode(std::make_unique<FBClearNode>("defaultFBClear", clearMask));
			mGraphicsEngine.getRenderGraph().addNode(std::make_unique<FBClearNode>("gFBClear", clearMask));
			mGraphicsEngine.getRenderGraph().addNode(std::make_unique<FBClearNode>("deferredFBClear", clearMask));
		}

		{	// Create the gBufferRenderer
			auto gBuffer = std::make_shared<FrameBuffer>();
			auto iGBufferResource = resources->addBindable(gBuffer);
			resources->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("gBuffer", resources, iGBufferResource) );

			auto gBufferRenderer = std::make_unique<Renderer3D>("gBufferRenderer");
			auto iGBufferBindable = gBufferRenderer->addBindable();
			gBufferRenderer->addInput( std::make_unique<BindableRNodeInput<FrameBuffer>>("gBuffer", gBufferRenderer.get(), iGBufferBindable) );
			gBufferRenderer->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("gBuffer", gBufferRenderer.get(), iGBufferBindable) );

			auto depthTexture = std::make_unique<Texture>(TextureTarget::Texture2D);
			depthTexture->setImage(nullptr, TypeId::Float, ColorFormat::Depth, ColorFormat::Depth24, width, height)
				.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
				.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
			gBuffer->attach(*depthTexture, FrameBufferAttachment::kDepth);
			auto iDepthTexBindable = gBufferRenderer->addBindable(std::move(depthTexture), false);
			gBufferRenderer->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("zBuffer", gBufferRenderer.get(), iDepthTexBindable) );

			auto positionTexture = std::make_unique<Texture>(TextureTarget::Texture2D);
			positionTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGB, ColorFormat::RGB16f, width, height)
				.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
				.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
			gBuffer->attach(*positionTexture, FrameBufferAttachment::kColor0);
			auto iPositionTexBindable = gBufferRenderer->addBindable(std::move(positionTexture), false);
			gBufferRenderer->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("position", gBufferRenderer.get(), iPositionTexBindable) );

			auto normalTexture = std::make_unique<Texture>(TextureTarget::Texture2D);
			normalTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGB, ColorFormat::RGB16f, width, height)
				.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
				.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
			gBuffer->attach(*normalTexture, FrameBufferAttachment::kColor0 + 1);
			auto iNormalTexBindable = gBufferRenderer->addBindable(std::move(normalTexture), false);
			gBufferRenderer->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("normal", gBufferRenderer.get(), iNormalTexBindable) );

			auto albedoTexture = std::make_unique<Texture>(TextureTarget::Texture2D);
			albedoTexture->setImage(nullptr, TypeId::UnsignedByte, ColorFormat::RGB, ColorFormat::RGB, width, height)
				.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
				.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
			gBuffer->attach(*albedoTexture, FrameBufferAttachment::kColor0 + 2);
			auto iAlbedoTexBindable = gBufferRenderer->addBindable(std::move(albedoTexture), false);
			gBufferRenderer->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("albedo", gBufferRenderer.get(), iAlbedoTexBindable) );

			auto materialTexture = std::make_unique<Texture>(TextureTarget::Texture2D);
			materialTexture->setImage(nullptr, TypeId::UnsignedByte, ColorFormat::RGB, ColorFormat::RGB, width, height)
				.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
				.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
			gBuffer->attach(*materialTexture, FrameBufferAttachment::kColor0 + 3);
			auto iMaterialTexBindable = gBufferRenderer->addBindable(std::move(materialTexture), false);
			gBufferRenderer->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("material", gBufferRenderer.get(), iMaterialTexBindable) );

			auto emissiveTexture = std::make_unique<Texture>(TextureTarget::Texture2D);
			emissiveTexture->setImage(nullptr, TypeId::UnsignedByte, ColorFormat::RGB, ColorFormat::RGB, width, height)
				.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
				.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
			gBuffer->attach(*emissiveTexture, FrameBufferAttachment::kColor0 + 4);
			auto iEmissiveTexBindable = gBufferRenderer->addBindable(std::move(emissiveTexture), false);
			gBufferRenderer->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("emissive", gBufferRenderer.get(), iEmissiveTexBindable) );

			mGraphicsEngine.getRenderGraph().addNode(std::move(gBufferRenderer));
		}

		{	// Create the rendererDeferredLight
			auto deferredBuffer = std::make_shared<FrameBuffer>();
			auto iDeferredBufferResource = resources->addBindable(deferredBuffer);
			resources->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("deferredBuffer", resources, iDeferredBufferResource) );

			auto rendererDeferredLight = std::make_unique<Renderer3D>("rendererDeferredLight");
			auto iTargetBindable = rendererDeferredLight->addBindable();
			auto iPositionTexBindable = rendererDeferredLight->addBindable();
			auto iNormalTexBindable = rendererDeferredLight->addBindable();
			auto iAlbedoTexBindable = rendererDeferredLight->addBindable();
			auto iMaterialTexBindable = rendererDeferredLight->addBindable();
			auto iEmissiveTexBindable = rendererDeferredLight->addBindable();
			rendererDeferredLight->addInput( std::make_unique<BindableRNodeInput<FrameBuffer>>("target", rendererDeferredLight.get(), iTargetBindable) );
			rendererDeferredLight->addInput( std::make_unique<BindableRNodeInput<Texture>>("position", rendererDeferredLight.get(), iPositionTexBindable) );
			rendererDeferredLight->addInput( std::make_unique<BindableRNodeInput<Texture>>("normal", rendererDeferredLight.get(), iNormalTexBindable) );
			rendererDeferredLight->addInput( std::make_unique<BindableRNodeInput<Texture>>("albedo", rendererDeferredLight.get(), iAlbedoTexBindable) );
			rendererDeferredLight->addInput( std::make_unique<BindableRNodeInput<Texture>>("material", rendererDeferredLight.get(), iMaterialTexBindable) );
			rendererDeferredLight->addInput( std::make_unique<BindableRNodeInput<Texture>>("emissive", rendererDeferredLight.get(), iEmissiveTexBindable) );

			auto depthTexture = std::make_unique<Texture>(TextureTarget::Texture2D);
			depthTexture->setImage(nullptr, TypeId::Float, ColorFormat::Depth, ColorFormat::Depth24, width, height)
				.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
				.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
			deferredBuffer->attach(*depthTexture, FrameBufferAttachment::kDepth);
			rendererDeferredLight->addBindable(std::move(depthTexture), false);

			auto colorTexture = std::make_unique<Texture>(TextureTarget::Texture2D);
			colorTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGBA, ColorFormat::RGBA16f, width, height)
				.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
				.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
			deferredBuffer->attach(*colorTexture, FrameBufferAttachment::kColor0);
			auto iColorTexBindable = rendererDeferredLight->addBindable(std::move(colorTexture), false);
			rendererDeferredLight->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("color", rendererDeferredLight.get(), iColorTexBindable) );

			auto brightTexture = std::make_unique<Texture>(TextureTarget::Texture2D);
			brightTexture->setImage(nullptr, TypeId::Float, ColorFormat::RGBA, ColorFormat::RGBA16f, width, height)
				.setWrapping(TextureWrap::ClampToEdge, TextureWrap::ClampToEdge)
				.setFiltering(TextureFilter::Linear, TextureFilter::Linear);
			deferredBuffer->attach(*brightTexture, FrameBufferAttachment::kColor0 + 1);
			auto iBrightTexBindable = rendererDeferredLight->addBindable(std::move(brightTexture), false);
			rendererDeferredLight->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("bright", rendererDeferredLight.get(), iBrightTexBindable) );

			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<TextureUnitNode>("defPositionTexUnitNode", kPosition) );
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<TextureUnitNode>("defNormalTexUnitNode", kNormal) );
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<TextureUnitNode>("defAlbedoTexUnitNode", kAlbedo) );
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<TextureUnitNode>("defMaterialTexUnitNode", kMaterial) );
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<TextureUnitNode>("defEmissiveTexUnitNode", kEmissive) );
			mGraphicsEngine.getRenderGraph().addNode( std::move(rendererDeferredLight) );
		}

		{	// Nodes used for blurring the bright colors (bloom)
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<GaussianBlurNode>("hBlurNode", application.getRepository(), mPlaneRenderable, width, height, true) );
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<GaussianBlurNode>("vBlurNode", application.getRepository(), mPlaneRenderable, width, height, false) );
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<TextureUnitNode>("hBlurTexUnitNode", GaussianBlurNode::kColorTextureUnit) );
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<TextureUnitNode>("vBlurTexUnitNode", GaussianBlurNode::kColorTextureUnit) );
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

					auto programCombineHDR = TechniqueLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentCombineHDR.glsl");
					if (!programCombineHDR) {
						throw std::runtime_error("programCombineHDR not found");
					}
					auto program = repository.add("programCombineHDR"s, std::move(programCombineHDR));

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

			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<TextureUnitNode>("combine0TexUnitNode", kColor0) );
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<TextureUnitNode>("combine1TexUnitNode", kColor1) );
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<CombineNode>("combineBloomNode", application.getRepository(), mPlaneRenderable) );
		}

		{	// Create the renderer2D
			auto renderer2D = std::make_unique<Renderer2D>("renderer2D");
			auto targetIndex = renderer2D->addBindable();
			renderer2D->addInput( std::make_unique<BindableRNodeInput<FrameBuffer>>("target", renderer2D.get(), targetIndex) );
			renderer2D->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("target", renderer2D.get(), targetIndex) );
			mGraphicsEngine.getRenderGraph().addNode( std::move(renderer2D) );
		}

		{	// Link the render graph nodes
			auto defaultFBClear = mGraphicsEngine.getRenderGraph().getNode("defaultFBClear"),
				gFBClear = mGraphicsEngine.getRenderGraph().getNode("gFBClear"),
				deferredFBClear = mGraphicsEngine.getRenderGraph().getNode("deferredFBClear"),
				gBufferRenderer = mGraphicsEngine.getRenderGraph().getNode("gBufferRenderer"),
				defPositionTexUnitNode = mGraphicsEngine.getRenderGraph().getNode("defPositionTexUnitNode"),
				defNormalTexUnitNode = mGraphicsEngine.getRenderGraph().getNode("defNormalTexUnitNode"),
				defAlbedoTexUnitNode = mGraphicsEngine.getRenderGraph().getNode("defAlbedoTexUnitNode"),
				defMaterialTexUnitNode = mGraphicsEngine.getRenderGraph().getNode("defMaterialTexUnitNode"),
				defEmissiveTexUnitNode = mGraphicsEngine.getRenderGraph().getNode("defEmissiveTexUnitNode"),
				rendererDeferredLight = mGraphicsEngine.getRenderGraph().getNode("rendererDeferredLight"),
				hBlurNode = mGraphicsEngine.getRenderGraph().getNode("hBlurNode"),
				vBlurNode = mGraphicsEngine.getRenderGraph().getNode("vBlurNode"),
				hBlurTexUnitNode = mGraphicsEngine.getRenderGraph().getNode("hBlurTexUnitNode"),
				vBlurTexUnitNode = mGraphicsEngine.getRenderGraph().getNode("vBlurTexUnitNode"),
				combine0TexUnitNode = mGraphicsEngine.getRenderGraph().getNode("combine0TexUnitNode"),
				combine1TexUnitNode = mGraphicsEngine.getRenderGraph().getNode("combine1TexUnitNode"),
				combineBloomNode = mGraphicsEngine.getRenderGraph().getNode("combineBloomNode"),
				renderer2D = mGraphicsEngine.getRenderGraph().getNode("renderer2D");

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
			hBlurTexUnitNode->findInput("input")->connect( rendererDeferredLight->findOutput("bright") );
			hBlurNode->findInput("input")->connect( hBlurTexUnitNode->findOutput("output") );
			vBlurTexUnitNode->findInput("input")->connect( hBlurNode->findOutput("output") );
			vBlurNode->findInput("input")->connect( vBlurTexUnitNode->findOutput("output") );
			combine0TexUnitNode->findInput("input")->connect( rendererDeferredLight->findOutput("color") );
			combine1TexUnitNode->findInput("input")->connect( vBlurNode->findOutput("output") );
			combineBloomNode->findInput("target")->connect( defaultFBClear->findOutput("output") );
			combineBloomNode->findInput("color0")->connect( combine0TexUnitNode->findOutput("output") );
			combineBloomNode->findInput("color1")->connect( combine1TexUnitNode->findOutput("output") );
			renderer2D->findInput("target")->connect( combineBloomNode->findOutput("target") );

			mGraphicsEngine.getRenderGraph().prepareGraph();
		}

		{	// Create the Techniques used for rendering to the framebuffers
			auto planeTechnique = std::make_unique<Technique>();

			// Create the pass and technique used for the deferred lighting
			auto rendererDeferredLight = dynamic_cast<Renderer3D*>(mGraphicsEngine.getRenderGraph().getNode("rendererDeferredLight"));

			auto programDeferredLighting = TechniqueLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentDeferredLighting.glsl");
			if (!programDeferredLighting) {
				throw std::runtime_error("programDeferredLighting not found");
			}
			auto program = application.getRepository().add("programDeferredLighting"s, std::move(programDeferredLighting));

			mLightingPass	= std::make_shared<Pass>(*rendererDeferredLight);
			mViewPosition	= std::make_shared<UniformVariableValue<glm::vec3>>("uViewPosition", *program, glm::vec3(0.0f));
			mNumLights	= std::make_shared<UniformVariableValue<unsigned int>>("uNumLights", *program, 0);
			mLightingPass->addBindable(program)
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
				.addBindable(mLightsBuffer)
				.addBindable(mNumLights)
				.addBindable(std::make_shared<UniformBlock>("LightsBlock", *program));
			planeTechnique->addPass(mLightingPass);

			mPlaneRenderable->addTechnique(std::move(planeTechnique));
			mGraphicsEngine.addRenderable(mPlaneRenderable.get());
		}

		{
			auto renderer2D = dynamic_cast<Renderer2D*>(mGraphicsEngine.getRenderGraph().getNode("renderer2D"));

			auto program = TechniqueLoader::createProgram("res/shaders/vertex2D.glsl", nullptr, "res/shaders/fragment2D.glsl");
			if (!program) {
				throw std::runtime_error("program2D couldn't be created");
			}
			auto program2D = application.getRepository().add("program2D"s, std::move(program));

			auto technique2D = std::make_unique<graphics::Technique>();
			technique2D->addPass( mCameraSystem.createPass2D(renderer2D, program2D) );
			application.getRepository().add("technique2D"s, std::move(technique2D));
		}
	}


	void AppRenderer::setIrradianceMap(TextureSPtr texture)
	{
		mLightingPass->removeBindable(mIrradianceMap);

		mIrradianceMap = texture;
		mIrradianceMap->setTextureUnit(kIrradianceMap);
		mLightingPass->addBindable(mIrradianceMap);
	}


	void AppRenderer::setPrefilterMap(TextureSPtr texture)
	{
		mLightingPass->removeBindable(mPrefilterMap);

		mPrefilterMap = texture;
		mPrefilterMap->setTextureUnit(kPrefilterMap);
		mLightingPass->addBindable(mPrefilterMap);
	}


	void AppRenderer::setBRDFMap(TextureSPtr texture)
	{
		mLightingPass->removeBindable(mBrdfMap);

		mBrdfMap = texture;
		mBrdfMap->setTextureUnit(kBRDFMap);
		mLightingPass->addBindable(mBrdfMap);
	}


	void AppRenderer::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the LightSources";

		unsigned int i = 0;
		std::array<ShaderLightSource, kMaxLights> uBaseLights;
		mEntityDatabase.iterateComponents<TransformsComponent, LightSource>(
			[&](Entity, TransformsComponent* transforms, LightSource* light) {
				if (i < kMaxLights) {
					uBaseLights[i].type = static_cast<unsigned int>(light->type);
					uBaseLights[i].position = transforms->position;
					uBaseLights[i].direction = glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f) * transforms->orientation);
					uBaseLights[i].color = { light->color, 1.0f };
					uBaseLights[i].intensity = light->intensity;
					switch (light->type) {
						case LightSource::Type::Directional: {
							uBaseLights[i].range = std::numeric_limits<float>::max();
						} break;
						case LightSource::Type::Point: {
							uBaseLights[i].range = light->range;
						} break;
						case LightSource::Type::Spot: {
							uBaseLights[i].range = light->range;
							uBaseLights[i].lightAngleScale = 1.0f / std::max(0.001f, std::cos(light->innerConeAngle) - std::cos(light->outerConeAngle));
							uBaseLights[i].lightAngleOffset = -std::cos(light->outerConeAngle) * uBaseLights[i].lightAngleScale;
						} break;
					}
					++i;
				}
			}
		);

		mNumLights->setValue(i);
		mLightsBuffer->copy(uBaseLights.data(), i);

		if (mCameraSystem.getActiveCamera() && mCameraSystem.wasCameraUpdated()) {
			mViewPosition->setValue(mCameraSystem.getActiveCamera()->getPosition());
		}

		SOMBRA_INFO_LOG << "Update end";
	}


	void AppRenderer::render()
	{
		SOMBRA_INFO_LOG << "Render start";
		mGraphicsEngine.render();
		SOMBRA_INFO_LOG << "Render end";
	}

}
