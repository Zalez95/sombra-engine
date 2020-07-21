#include <array>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "se/app/GraphicsManager.h"
#include "se/app/events/ResizeEvent.h"
#include "se/app/loaders/MeshLoader.h"
#include "se/app/loaders/TechniqueLoader.h"
#include "se/app/graphics/GaussianBlurNode.h"
#include "se/utils/Log.h"
#include "se/graphics/Pass.h"
#include "se/graphics/Technique.h"
#include "se/graphics/FBClearNode.h"
#include "se/graphics/FBCopyNode.h"
#include "se/graphics/TextureUnitNode.h"
#include "se/graphics/2D/Renderer2D.h"
#include "se/graphics/3D/Renderer3D.h"
#include "se/graphics/core/Program.h"
#include "se/graphics/core/Texture.h"
#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/GraphicsOperations.h"

namespace se::app {

	struct GraphicsManager::ShaderLightSource
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


	struct GraphicsManager::RenderableMeshData
	{
		RenderableMeshUPtr renderable;
		std::vector<std::shared_ptr<graphics::UniformVariableValue<glm::mat4>>> modelMatrix;
		SkinSPtr skin;
		std::vector<std::shared_ptr<graphics::UniformVariableValueVector<glm::mat4, kMaxJoints>>> jointMatrices;

		RenderableMeshData(RenderableMeshUPtr renderable) :
			renderable(std::move(renderable)), skin(nullptr) {};
	};


	struct GraphicsManager::RenderableTerrainData
	{
		RenderableTerrainUPtr renderable;
		std::vector<std::shared_ptr<graphics::UniformVariableValue<glm::mat4>>> modelMatrix;

		RenderableTerrainData(RenderableTerrainUPtr renderable) :
			renderable(std::move(renderable)) {};
	};


	struct GraphicsManager::PassData
	{
		std::shared_ptr<graphics::Pass> pass;
		std::shared_ptr<graphics::Program> program;
		std::shared_ptr<graphics::UniformVariableValue<glm::mat4>> viewMatrix;
		std::shared_ptr<graphics::UniformVariableValue<glm::mat4>> projectionMatrix;
	};


	struct GraphicsManager::Impl
	{
		static constexpr int kPosition			= 0;
		static constexpr int kNormal			= 1;
		static constexpr int kAlbedo			= 2;
		static constexpr int kMaterial			= 3;
		static constexpr int kEmissive			= 4;
		static constexpr int kIrradianceMap		= 5;
		static constexpr int kPrefilterMap		= 6;
		static constexpr int kBRDFMap			= 7;

		static constexpr int kColor				= 0;
		static constexpr int kBright			= 1;

		Camera* activeCamera;

		// Defferred lighting data
		std::shared_ptr<graphics::Pass> lightingPass;
		std::shared_ptr<graphics::UniformVariableValue<glm::vec3>> viewPosition;
		std::shared_ptr<graphics::UniformVariableValue<unsigned int>> numLights;
		std::shared_ptr<graphics::UniformBuffer> lightsBuffer;
		std::shared_ptr<graphics::Texture> irradianceMap, prefilterMap, brdfMap;
		std::shared_ptr<graphics::RenderableMesh> planeRenderable;

		std::map<Entity*, CameraUPtr> cameraEntities;
		std::map<Entity*, LightSourceUPtr> lightEntities;
		std::multimap<Entity*, RenderableMeshData> renderableMeshEntities;
		std::map<Entity*, RenderableTerrainData> renderableTerrainEntities;
		std::vector<PassData> passesData;

		Impl() : activeCamera(nullptr)
		{
			lightsBuffer = std::make_shared<graphics::UniformBuffer>();
			utils::FixedVector<ShaderLightSource, kMaxLights> lightsBufferData(kMaxLights);
			lightsBuffer->resizeAndCopy(lightsBufferData.data(), lightsBufferData.size());

			// Create the plane used for rendendering the framebuffers
			RawMesh planeRawMesh;
			planeRawMesh.positions = { {-1.0f,-1.0f, 0.0f }, { 1.0f,-1.0f, 0.0f }, {-1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f } };
			planeRawMesh.faceIndices = { 0, 1, 2, 1, 3, 2, };
			auto planeMesh = std::make_unique<graphics::Mesh>(MeshLoader::createGraphicsMesh(planeRawMesh));
			planeRenderable = std::make_shared<graphics::RenderableMesh>(std::move(planeMesh));
		};
	};


	GraphicsManager::GraphicsManager(
		graphics::GraphicsEngine& graphicsEngine, EventManager& eventManager, std::size_t width, std::size_t height
	) : mGraphicsEngine(graphicsEngine), mEventManager(eventManager)
	{
		mEventManager.subscribe(this, Topic::Resize);

		SOMBRA_INFO_LOG << graphics::GraphicsOperations::getGraphicsInfo();

		mImpl = std::make_unique<Impl>();

		{	// Create the FBClearNodes
			auto clearMask = graphics::FrameBufferMask::Mask().set(graphics::FrameBufferMask::kColor).set(graphics::FrameBufferMask::kDepth);
			mGraphicsEngine.getRenderGraph().addNode(std::make_unique<graphics::FBClearNode>("defaultFBClear", clearMask));
			mGraphicsEngine.getRenderGraph().addNode(std::make_unique<graphics::FBClearNode>("gFBClear", clearMask));
			mGraphicsEngine.getRenderGraph().addNode(std::make_unique<graphics::FBClearNode>("deferredFBClear", clearMask));
		}

		{	// Create the gBufferRenderer
			auto resources = dynamic_cast<graphics::BindableRenderNode*>(mGraphicsEngine.getRenderGraph().getNode("resources"));

			auto gBuffer = std::make_shared<graphics::FrameBuffer>();
			auto iGBufferResource = resources->addBindable(gBuffer);
			resources->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::FrameBuffer>>("gBuffer", resources, iGBufferResource) );

			auto gBufferRenderer = std::make_unique<graphics::Renderer3D>("gBufferRenderer");
			auto iGBufferBindable = gBufferRenderer->addBindable();
			gBufferRenderer->addInput( std::make_unique<graphics::BindableRNodeInput<graphics::FrameBuffer>>("gBuffer", gBufferRenderer.get(), iGBufferBindable) );
			gBufferRenderer->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::FrameBuffer>>("gBuffer", gBufferRenderer.get(), iGBufferBindable) );

			auto depthTexture = std::make_unique<graphics::Texture>(graphics::TextureTarget::Texture2D);
			depthTexture->setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::Depth, graphics::ColorFormat::Depth24, width, height)
				.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
				.setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear);
			gBuffer->attach(*depthTexture, graphics::FrameBufferAttachment::kDepth);
			auto iDepthTexBindable = gBufferRenderer->addBindable(std::move(depthTexture), false);
			gBufferRenderer->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("zBuffer", gBufferRenderer.get(), iDepthTexBindable) );

			auto positionTexture = std::make_unique<graphics::Texture>(graphics::TextureTarget::Texture2D);
			positionTexture->setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::RGB, graphics::ColorFormat::RGB16f, width, height)
				.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
				.setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear);
			gBuffer->attach(*positionTexture, graphics::FrameBufferAttachment::kColor0);
			auto iPositionTexBindable = gBufferRenderer->addBindable(std::move(positionTexture), false);
			gBufferRenderer->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("position", gBufferRenderer.get(), iPositionTexBindable) );

			auto normalTexture = std::make_unique<graphics::Texture>(graphics::TextureTarget::Texture2D);
			normalTexture->setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::RGB, graphics::ColorFormat::RGB16f, width, height)
				.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
				.setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear);
			gBuffer->attach(*normalTexture, graphics::FrameBufferAttachment::kColor0 + 1);
			auto iNormalTexBindable = gBufferRenderer->addBindable(std::move(normalTexture), false);
			gBufferRenderer->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("normal", gBufferRenderer.get(), iNormalTexBindable) );

			auto albedoTexture = std::make_unique<graphics::Texture>(graphics::TextureTarget::Texture2D);
			albedoTexture->setImage(nullptr, graphics::TypeId::UnsignedByte, graphics::ColorFormat::RGB, graphics::ColorFormat::RGB, width, height)
				.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
				.setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear);
			gBuffer->attach(*albedoTexture, graphics::FrameBufferAttachment::kColor0 + 2);
			auto iAlbedoTexBindable = gBufferRenderer->addBindable(std::move(albedoTexture), false);
			gBufferRenderer->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("albedo", gBufferRenderer.get(), iAlbedoTexBindable) );

			auto materialTexture = std::make_unique<graphics::Texture>(graphics::TextureTarget::Texture2D);
			materialTexture->setImage(nullptr, graphics::TypeId::UnsignedByte, graphics::ColorFormat::RGB, graphics::ColorFormat::RGB, width, height)
				.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
				.setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear);
			gBuffer->attach(*materialTexture, graphics::FrameBufferAttachment::kColor0 + 3);
			auto iMaterialTexBindable = gBufferRenderer->addBindable(std::move(materialTexture), false);
			gBufferRenderer->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("material", gBufferRenderer.get(), iMaterialTexBindable) );

			auto emissiveTexture = std::make_unique<graphics::Texture>(graphics::TextureTarget::Texture2D);
			emissiveTexture->setImage(nullptr, graphics::TypeId::UnsignedByte, graphics::ColorFormat::RGB, graphics::ColorFormat::RGB, width, height)
				.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
				.setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear);
			gBuffer->attach(*emissiveTexture, graphics::FrameBufferAttachment::kColor0 + 4);
			auto iEmissiveTexBindable = gBufferRenderer->addBindable(std::move(emissiveTexture), false);
			gBufferRenderer->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("emissive", gBufferRenderer.get(), iEmissiveTexBindable) );

			mGraphicsEngine.getRenderGraph().addNode(std::move(gBufferRenderer));
		}

		{	// Create the rendererDeferredLight
			auto resources = dynamic_cast<graphics::BindableRenderNode*>(mGraphicsEngine.getRenderGraph().getNode("resources"));

			auto deferredBuffer = std::make_shared<graphics::FrameBuffer>();
			auto iDeferredBufferResource = resources->addBindable(deferredBuffer);
			resources->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::FrameBuffer>>("deferredBuffer", resources, iDeferredBufferResource) );

			auto rendererDeferredLight = std::make_unique<graphics::Renderer3D>("rendererDeferredLight");
			auto iTargetBindable = rendererDeferredLight->addBindable();
			auto iPositionTexBindable = rendererDeferredLight->addBindable();
			auto iNormalTexBindable = rendererDeferredLight->addBindable();
			auto iAlbedoTexBindable = rendererDeferredLight->addBindable();
			auto iMaterialTexBindable = rendererDeferredLight->addBindable();
			auto iEmissiveTexBindable = rendererDeferredLight->addBindable();
			rendererDeferredLight->addInput( std::make_unique<graphics::BindableRNodeInput<graphics::FrameBuffer>>("target", rendererDeferredLight.get(), iTargetBindable) );
			rendererDeferredLight->addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("position", rendererDeferredLight.get(), iPositionTexBindable) );
			rendererDeferredLight->addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("normal", rendererDeferredLight.get(), iNormalTexBindable) );
			rendererDeferredLight->addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("albedo", rendererDeferredLight.get(), iAlbedoTexBindable) );
			rendererDeferredLight->addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("material", rendererDeferredLight.get(), iMaterialTexBindable) );
			rendererDeferredLight->addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("emissive", rendererDeferredLight.get(), iEmissiveTexBindable) );

			auto depthTexture = std::make_unique<graphics::Texture>(graphics::TextureTarget::Texture2D);
			depthTexture->setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::Depth, graphics::ColorFormat::Depth24, width, height)
				.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
				.setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear);
			deferredBuffer->attach(*depthTexture, graphics::FrameBufferAttachment::kDepth);
			rendererDeferredLight->addBindable(std::move(depthTexture), false);

			auto colorTexture = std::make_unique<graphics::Texture>(graphics::TextureTarget::Texture2D);
			colorTexture->setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::RGBA, graphics::ColorFormat::RGBA16f, width, height)
				.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
				.setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear);
			deferredBuffer->attach(*colorTexture, graphics::FrameBufferAttachment::kColor0);
			auto iColorTexBindable = rendererDeferredLight->addBindable(std::move(colorTexture), false);
			rendererDeferredLight->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("color", rendererDeferredLight.get(), iColorTexBindable) );

			auto brightTexture = std::make_unique<graphics::Texture>(graphics::TextureTarget::Texture2D);
			brightTexture->setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::RGBA, graphics::ColorFormat::RGBA16f, width, height)
				.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
				.setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear);
			deferredBuffer->attach(*brightTexture, graphics::FrameBufferAttachment::kColor0 + 1);
			auto iBrightTexBindable = rendererDeferredLight->addBindable(std::move(brightTexture), false);
			rendererDeferredLight->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("bright", rendererDeferredLight.get(), iBrightTexBindable) );

			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<graphics::TextureUnitNode>("defPositionTexUnitNode", Impl::kPosition) );
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<graphics::TextureUnitNode>("defNormalTexUnitNode", Impl::kNormal) );
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<graphics::TextureUnitNode>("defAlbedoTexUnitNode", Impl::kAlbedo) );
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<graphics::TextureUnitNode>("defMaterialTexUnitNode", Impl::kMaterial) );
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<graphics::TextureUnitNode>("defEmissiveTexUnitNode", Impl::kEmissive) );
			mGraphicsEngine.getRenderGraph().addNode( std::move(rendererDeferredLight) );
		}

		{	// Nodes used for blurring the bright colors (bloom)
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<GaussianBlurNode>("hBlurNode", *this, mImpl->planeRenderable, width, height, true) );
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<GaussianBlurNode>("vBlurNode", *this, mImpl->planeRenderable, width, height, false) );
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<graphics::TextureUnitNode>("hBlurTexUnitNode", GaussianBlurNode::kColorTextureUnit) );
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<graphics::TextureUnitNode>("vBlurTexUnitNode", GaussianBlurNode::kColorTextureUnit) );
		}

		{	// Node used for combining the bloom and color
			static constexpr int kColor0 = 0;
			static constexpr int kColor1 = 1;

			class CombineNode : public graphics::BindableRenderNode
			{
			private:
				std::shared_ptr<graphics::RenderableMesh> mPlane;
			public:
				CombineNode(const std::string& name, GraphicsManager& graphicsManager, std::shared_ptr<graphics::RenderableMesh> plane) :
					BindableRenderNode(name), mPlane(plane)
				{
					auto iTargetBindable = addBindable();
					addInput( std::make_unique<graphics::BindableRNodeInput<graphics::FrameBuffer>>("target", this, iTargetBindable) );
					addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("color0", this, addBindable()) );
					addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("color1", this, addBindable()) );
					addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::FrameBuffer>>("target", this, iTargetBindable) );

					auto programCombineHDR = TechniqueLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentCombineHDR.glsl");
					if (!programCombineHDR) {
						throw std::runtime_error("programCombineHDR not found");
					}
					auto program = graphicsManager.getProgramRepository().add("programCombineHDR", std::move(programCombineHDR));

					addBindable(program);
					addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", *program, glm::mat4(1.0f)));
					addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix", *program, glm::mat4(1.0f)));
					addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", *program, glm::mat4(1.0f)));
					addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uColor0", *program, kColor0));
					addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uColor1", *program, kColor1));
				};

				virtual void execute() override
				{
					bind();
					mPlane->bind();
					mPlane->draw();
				};
			};

			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<graphics::TextureUnitNode>("combine0TexUnitNode", kColor0) );
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<graphics::TextureUnitNode>("combine1TexUnitNode", kColor1) );
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<CombineNode>("combineBloomNode", *this, mImpl->planeRenderable) );
		}

		{	// Create the renderer2D
			auto renderer2D = std::make_unique<graphics::Renderer2D>("renderer2D");
			auto targetIndex = renderer2D->addBindable();
			renderer2D->addInput( std::make_unique<graphics::BindableRNodeInput<graphics::FrameBuffer>>("target", renderer2D.get(), targetIndex) );
			renderer2D->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::FrameBuffer>>("target", renderer2D.get(), targetIndex) );
			mGraphicsEngine.getRenderGraph().addNode( std::move(renderer2D) );
		}

		{	// Link the render graph nodes
			auto resources = mGraphicsEngine.getRenderGraph().getNode("resources"),
				defaultFBClear = mGraphicsEngine.getRenderGraph().getNode("defaultFBClear"),
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
			auto planeTechnique = std::make_unique<graphics::Technique>();

			// Create the pass and technique used for the deferred lighting
			auto rendererDeferredLight = dynamic_cast<graphics::Renderer3D*>(mGraphicsEngine.getRenderGraph().getNode("rendererDeferredLight"));

			auto programDeferredLighting = TechniqueLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentDeferredLighting.glsl");
			if (!programDeferredLighting) {
				throw std::runtime_error("programDeferredLighting not found");
			}
			auto program = mProgramRepository.add("programDeferredLighting", std::move(programDeferredLighting));

			mImpl->lightingPass	= std::make_shared<graphics::Pass>(*rendererDeferredLight);
			mImpl->viewPosition	= std::make_shared<graphics::UniformVariableValue<glm::vec3>>("uViewPosition", *program, glm::vec3(0.0f));
			mImpl->numLights	= std::make_shared<graphics::UniformVariableValue<unsigned int>>("uNumLights", *program, 0);
			mImpl->lightingPass->addBindable(program)
				.addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", *program, glm::mat4(1.0f)))
				.addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix", *program, glm::mat4(1.0f)))
				.addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", *program, glm::mat4(1.0f)))
				.addBindable(mImpl->viewPosition)
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uPosition", *program, Impl::kPosition))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uNormal", *program, Impl::kNormal))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uAlbedo", *program, Impl::kAlbedo))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uMaterial", *program, Impl::kMaterial))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uEmissive", *program, Impl::kEmissive))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uIrradianceMap", *program, Impl::kIrradianceMap))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uPrefilterMap", *program, Impl::kPrefilterMap))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uBRDFMap", *program, Impl::kBRDFMap))
				.addBindable(mImpl->lightsBuffer)
				.addBindable(mImpl->numLights)
				.addBindable(std::make_shared<graphics::UniformBlock>("LightsBlock", *program));
			planeTechnique->addPass(mImpl->lightingPass);

			mImpl->planeRenderable->addTechnique(std::move(planeTechnique));
			mGraphicsEngine.addRenderable(mImpl->planeRenderable.get());
		}
	}


	GraphicsManager::~GraphicsManager()
	{
		// Remove all entities
		for (auto& pair : mImpl->cameraEntities) {
			removeEntity(pair.first);
		}
		for (auto& pair : mImpl->lightEntities) {
			removeEntity(pair.first);
		}
		for (auto& pair : mImpl->renderableMeshEntities) {
			removeEntity(pair.first);
		}
		for (auto& pair : mImpl->renderableTerrainEntities) {
			removeEntity(pair.first);
		}

		// Clear the GraphicsManager data
		mGraphicsEngine.removeRenderable(mImpl->planeRenderable.get());

		mEventManager.unsubscribe(this, Topic::Resize);
	}


	void GraphicsManager::notify(const IEvent& event)
	{
		tryCall(&GraphicsManager::onResizeEvent, event);
	}


	void GraphicsManager::addCameraEntity(Entity* entity, CameraUPtr camera)
	{
		if (!entity || !camera) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as Camera";
			return;
		}

		// The Camera initial data is overridden by the entity one
		camera->setPosition(entity->position);
		camera->setTarget(entity->position + glm::vec3(0.0f, 0.0f, 1.0f) * entity->orientation);
		camera->setUp({ 0.0f, 1.0f, 0.0f });

		// Add the Camera
		Camera* cPtr = camera.get();
		mImpl->cameraEntities.emplace(entity, std::move(camera));
		mImpl->activeCamera = cPtr;
		SOMBRA_INFO_LOG << "Entity " << entity << " with Camera " << cPtr << " added successfully";
	}


	void GraphicsManager::addLightEntity(Entity* entity, LightSourceUPtr lightSource)
	{
		if (!entity || !lightSource) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as LightSource";
			return;
		}

		// Add the ILight
		LightSource* lPtr = lightSource.get();
		mImpl->lightEntities.emplace(entity, std::move(lightSource));
		SOMBRA_INFO_LOG << "Entity " << entity << " with LightSource " << lPtr << " added successfully";
	}


	void GraphicsManager::setIrradianceMap(TextureSPtr texture)
	{
		mImpl->lightingPass->removeBindable(mImpl->irradianceMap);

		mImpl->irradianceMap = texture;
		mImpl->irradianceMap->setTextureUnit(Impl::kIrradianceMap);
		mImpl->lightingPass->addBindable(mImpl->irradianceMap);
	}


	void GraphicsManager::setPrefilterMap(TextureSPtr texture)
	{
		mImpl->lightingPass->removeBindable(mImpl->prefilterMap);

		mImpl->prefilterMap = texture;
		mImpl->prefilterMap->setTextureUnit(Impl::kPrefilterMap);
		mImpl->lightingPass->addBindable(texture);
	}


	void GraphicsManager::setBRDFMap(TextureSPtr texture)
	{
		mImpl->lightingPass->removeBindable(mImpl->brdfMap);

		mImpl->brdfMap = texture;
		mImpl->brdfMap->setTextureUnit(Impl::kBRDFMap);
		mImpl->lightingPass->addBindable(mImpl->brdfMap);
	}


	GraphicsManager::PassSPtr GraphicsManager::createPass2D(ProgramSPtr program)
	{
		auto renderer2D = dynamic_cast<graphics::Renderer2D*>( mGraphicsEngine.getRenderGraph().getNode("renderer2D") );
		if (!renderer2D) {
			return nullptr;
		}

		auto pass = std::make_shared<graphics::Pass>(*renderer2D);
		pass->addBindable(program)
			.addBindable(std::make_shared<graphics::BlendingOperation>(true))
			.addBindable(std::make_shared<graphics::DepthTestOperation>(false));

		for (int i = 0; i < static_cast<int>(graphics::Renderer2D::kMaxTextures); ++i) {
			utils::ArrayStreambuf<char, 64> aStreambuf;
			std::ostream(&aStreambuf) << "uTextures[" << i << "]";
			pass->addBindable(std::make_shared<graphics::UniformVariableValue<int>>(aStreambuf.data(), *program, i));
		}

		// FIXME: cache this
		pass->addBindable(std::make_shared<graphics::UniformVariableCallback<glm::mat4>>(
			"uProjectionMatrix", *program,
			[]() {
				int x, y;
				std::size_t width, height;
				graphics::GraphicsOperations::getViewport(x, y, width, height);
				return glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, -1.0f, 1.0f);
			}
		));

		return pass;
	}


	GraphicsManager::PassSPtr GraphicsManager::createPass3D(graphics::Renderer* renderer, ProgramSPtr program, bool addProgram)
	{
		auto pass = std::make_shared<graphics::Pass>(*renderer);
		auto& passData = mImpl->passesData.emplace_back();
		passData.pass = pass;
		passData.program = program;

		if (addProgram) {
			pass->addBindable(program);
		}

		passData.viewMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix", *program);
		passData.projectionMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", *program);
		if (mImpl->activeCamera) {
			passData.viewMatrix->setValue(mImpl->activeCamera->getViewMatrix());
			passData.projectionMatrix->setValue(mImpl->activeCamera->getProjectionMatrix());
		}

		pass->addBindable(passData.viewMatrix)
			.addBindable(passData.projectionMatrix);

		return pass;
	}


	void GraphicsManager::addMeshEntity(Entity* entity, RenderableMeshUPtr renderable, SkinSPtr skin)
	{
		if (!entity || !renderable) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as Mesh";
			return;
		}

		graphics::RenderableMesh* rPtr = renderable.get();
		auto& meshData = mImpl->renderableMeshEntities.emplace(entity, std::move(renderable))->second;

		// Add the RenderableMesh model matrix uniform
		glm::mat4 translation	= glm::translate(glm::mat4(1.0f), entity->position);
		glm::mat4 rotation		= glm::mat4_cast(entity->orientation);
		glm::mat4 scale			= glm::scale(glm::mat4(1.0f), entity->scale);
		glm::mat4 modelMatrix	= translation * rotation * scale;

		rPtr->processTechniques([&](auto technique) { technique->processPasses([&](auto pass) {
			auto itPassData = std::find_if(mImpl->passesData.begin(), mImpl->passesData.end(), [&](const PassData& passData) {
				return passData.pass == pass;
			});
			if (itPassData != mImpl->passesData.end()) {
				rPtr->addBindable(
					meshData.modelMatrix.emplace_back(
						std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", *itPassData->program, modelMatrix)
					)
				);

				if (skin) {
					auto jointMatrices = calculateJointMatrices(*skin, modelMatrix);
					std::size_t numJoints = std::min(jointMatrices.size(), static_cast<std::size_t>(kMaxJoints));

					rPtr->addBindable(
						meshData.jointMatrices.emplace_back(
							std::make_shared<graphics::UniformVariableValueVector<glm::mat4, kMaxJoints>>(
								"uJointMatrices", *itPassData->program, jointMatrices.data(), numJoints
							)
						)
					);
				}
			}
			else {
				SOMBRA_WARN_LOG << "RenderableMesh has a Pass " << pass << " not added to the GraphicsManager";
			}
		}); });

		mGraphicsEngine.addRenderable(rPtr);

		SOMBRA_INFO_LOG << "Entity " << entity << " with RenderableMesh " << rPtr << " added successfully";
	}


	void GraphicsManager::addTerrainEntity(Entity* entity, RenderableTerrainUPtr renderable)
	{
		if (!entity || !renderable) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as RenderableTerrain";
			return;
		}

		graphics::RenderableTerrain* rPtr = renderable.get();
		auto pair = mImpl->renderableTerrainEntities.emplace(entity, std::move(renderable));
		if (!pair.second) {
			SOMBRA_WARN_LOG << "Entity " << entity << " already has a RenderableTerrain";
			return;
		}

		auto& terrainData = pair.first->second;
		if (mImpl->activeCamera) {
			rPtr->setHighestLodLocation(mImpl->activeCamera->getPosition());
		}

		// Add the RenderableTerrain model matrix uniform
		glm::mat4 translation	= glm::translate(glm::mat4(1.0f), entity->position);
		glm::mat4 rotation		= glm::mat4_cast(entity->orientation);
		glm::mat4 modelMatrix	= translation * rotation;

		rPtr->processTechniques([&](auto technique) { technique->processPasses([&](auto pass) {
			auto itPassData = std::find_if(mImpl->passesData.begin(), mImpl->passesData.end(), [&](const PassData& passData) {
				return passData.pass == pass;
			});
			if (itPassData != mImpl->passesData.end()) {
				rPtr->addBindable(
					terrainData.modelMatrix.emplace_back(
						std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", *itPassData->program, modelMatrix)
					)
				);
			}
			else {
				SOMBRA_WARN_LOG << "RenderableTerrain has a Pass " << pass << " not added to the GraphicsManager";
			}
		}); });

		mGraphicsEngine.addRenderable(rPtr);

		SOMBRA_INFO_LOG << "Entity " << entity << " with RenderableTerrain " << rPtr << " added successfully";
	}


	void GraphicsManager::removeEntity(Entity* entity)
	{
		auto itCamera = mImpl->cameraEntities.find(entity);
		if (itCamera != mImpl->cameraEntities.end()) {
			if (itCamera->second.get() == mImpl->activeCamera) {
				mImpl->activeCamera = nullptr;
			}
			mImpl->cameraEntities.erase(itCamera);
			SOMBRA_INFO_LOG << "Camera Entity " << entity << " removed successfully";
		}

		auto itLight = mImpl->lightEntities.find(entity);
		if (itLight != mImpl->lightEntities.end()) {
			mImpl->lightEntities.erase(itLight);
			SOMBRA_INFO_LOG << "ILight Entity " << entity << " removed successfully";
		}

		auto [itRMeshBegin, itRMeshEnd] = mImpl->renderableMeshEntities.equal_range(entity);
		for (auto itRMesh = itRMeshBegin; itRMesh != itRMeshEnd;) {
			mGraphicsEngine.removeRenderable(itRMesh->second.renderable.get());
			itRMesh = mImpl->renderableMeshEntities.erase(itRMesh);
			SOMBRA_INFO_LOG << "Mesh Entity " << entity << " removed successfully";
		}

		auto itRenderableTerrain = mImpl->renderableTerrainEntities.find(entity);
		if (itRenderableTerrain != mImpl->renderableTerrainEntities.end()) {
			mGraphicsEngine.removeRenderable(itRenderableTerrain->second.renderable.get());
			mImpl->renderableTerrainEntities.erase(itRenderableTerrain);
			SOMBRA_INFO_LOG << "RenderableTerrain Entity " << entity << " removed successfully";
		}
	}


	void GraphicsManager::update()
	{
		SOMBRA_INFO_LOG << "Update start";

		SOMBRA_DEBUG_LOG << "Updating the Cameras";
		bool activeCameraUpdated = false;
		for (auto& [entity, camera] : mImpl->cameraEntities) {
			if (entity->updated.any()) {
				camera->setPosition(entity->position);
				camera->setTarget(entity->position + glm::vec3(0.0f, 0.0f, 1.0f) * entity->orientation);
				camera->setUp({ 0.0f, 1.0f, 0.0f });

				if (camera.get() == mImpl->activeCamera) {
					activeCameraUpdated = true;
				}
			}
		}

		if (activeCameraUpdated) {
			mImpl->viewPosition->setValue( mImpl->activeCamera->getPosition() );
			for (auto& passData : mImpl->passesData) {
				passData.viewMatrix->setValue(mImpl->activeCamera->getViewMatrix());
				passData.projectionMatrix->setValue(mImpl->activeCamera->getProjectionMatrix());
			}
		}

		SOMBRA_DEBUG_LOG << "Updating the LightSources";
		unsigned int uNumLights = std::min(static_cast<unsigned int>(mImpl->lightEntities.size()), kMaxLights);
		std::array<ShaderLightSource, kMaxLights> uBaseLights;

		unsigned int i = 0;
		for (auto& [entity, light] : mImpl->lightEntities) {
			uBaseLights[i].type = static_cast<unsigned int>(light->type);
			uBaseLights[i].position = entity->position;
			uBaseLights[i].direction = glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f) * entity->orientation);
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

			if (++i >= uNumLights) { break; }
		}

		mImpl->numLights->setValue(uNumLights);
		mImpl->lightsBuffer->copy(uBaseLights.data(), uBaseLights.size());

		SOMBRA_DEBUG_LOG << "Updating the Meshes";
		for (auto& [entity, meshData] : mImpl->renderableMeshEntities) {
			if (entity->updated.any()) {
				glm::mat4 translation	= glm::translate(glm::mat4(1.0f), entity->position);
				glm::mat4 rotation		= glm::mat4_cast(entity->orientation);
				glm::mat4 scale			= glm::scale(glm::mat4(1.0f), entity->scale);
				glm::mat4 modelMatrix	= translation * rotation * scale;

				for(auto& uniform : meshData.modelMatrix) {
					uniform->setValue(modelMatrix);
				}

				if (meshData.skin) {
					auto jointMatrices = calculateJointMatrices(*meshData.skin, modelMatrix);
					std::size_t numJoints = std::min(jointMatrices.size(), static_cast<std::size_t>(kMaxJoints));

					for(auto& uniform : meshData.jointMatrices) {
						uniform->setValue(jointMatrices.data(), numJoints);
					}
				}
			}
		}

		SOMBRA_DEBUG_LOG << "Updating the RenderableTerrains";
		for (auto& [entity, terrainData] : mImpl->renderableTerrainEntities) {
			if (entity->updated.any()) {
				glm::mat4 translation	= glm::translate(glm::mat4(1.0f), entity->position);
				glm::mat4 rotation		= glm::mat4_cast(entity->orientation);
				glm::mat4 modelMatrix	= translation * rotation;

				for(auto& uniform : terrainData.modelMatrix) {
					uniform->setValue(modelMatrix);
				}
			}

			if (activeCameraUpdated) {
				terrainData.renderable->setHighestLodLocation(mImpl->activeCamera->getPosition());
			}
		}

		SOMBRA_INFO_LOG << "Update end";
	}


	void GraphicsManager::render()
	{
		SOMBRA_INFO_LOG << "Render start";
		mGraphicsEngine.render();
		SOMBRA_INFO_LOG << "Render end";
	}

// Private functions
	void GraphicsManager::onResizeEvent(const ResizeEvent& event)
	{
		auto width = static_cast<std::size_t>(event.getWidth());
		auto height = static_cast<std::size_t>(event.getHeight());

		graphics::GraphicsOperations::setViewport(0, 0, width, height);
	}

}
