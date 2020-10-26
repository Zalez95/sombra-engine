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

	struct AppRenderer::EnvironmentTexUnits
	{
		static constexpr int kIrradianceMap	= 0;
		static constexpr int kPrefilterMap	= 1;
		static constexpr int kBRDFMap		= 2;
		static constexpr int kShadowMap		= 3;
	};


	class AppRenderer::DeferredLightRenderer : public BindableRenderNode
	{
	public:
		struct ShaderLightSource
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

		struct GBufferTexUnits
		{
			static constexpr int kPosition		= 4;
			static constexpr int kNormal		= 5;
			static constexpr int kAlbedo		= 6;
			static constexpr int kMaterial		= 7;
			static constexpr int kEmissive		= 8;
		};

		/** The maximum number of lights in the program */
		static constexpr unsigned int kMaxLights = 32;

	private:
		/** The plane used for rendering */
		std::shared_ptr<RenderableMesh> mPlane;

		/** The uniform variable that the Camera location in world space */
		std::shared_ptr<graphics::UniformVariableValue<glm::vec3>> mViewPosition;

		/** The uniform variable that holds the number of active lights to
		 * render */
		std::shared_ptr<graphics::UniformVariableValue<unsigned int>> mNumLights;

		/** The UniformBuffer where the lights data will be stored */
		std::shared_ptr<graphics::UniformBuffer> mLightsBuffer;

		/** The uniform variable that holds the index of the LightSource used
		 * for rendering the Shadows */
		std::shared_ptr<graphics::UniformVariableValue<unsigned int>> mShadowLightIndex;

		/** The uniform variable with the view matrix of the shadow mapping */
		std::shared_ptr<graphics::UniformVariableValue<glm::mat4>> mShadowViewMatrix;

		/** The uniform variable with the projection matrix of the shadow
		 * mapping */
		std::shared_ptr<graphics::UniformVariableValue<glm::mat4>> mShadowProjectionMatrix;

	public:
		DeferredLightRenderer(
			const std::string& name, utils::Repository& repository,
			std::shared_ptr<RenderableMesh> plane
		) : BindableRenderNode(name), mPlane(plane)
		{
			auto iTargetBindable = addBindable();
			addInput( std::make_unique<BindableRNodeInput<FrameBuffer>>("target", this, iTargetBindable) );
			addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("target", this, iTargetBindable) );

			auto iIrradianceTexBindable = addBindable();
			auto iPrefilterTexBindable = addBindable();
			auto iBRDFTexBindable = addBindable();
			auto iShadowTexBindable = addBindable();
			auto iPositionTexBindable = addBindable();
			auto iNormalTexBindable = addBindable();
			auto iAlbedoTexBindable = addBindable();
			auto iMaterialTexBindable = addBindable();
			auto iEmissiveTexBindable = addBindable();
			addInput( std::make_unique<BindableRNodeInput<Texture>>("irradiance", this, iIrradianceTexBindable) );
			addInput( std::make_unique<BindableRNodeInput<Texture>>("prefilter", this, iPrefilterTexBindable) );
			addInput( std::make_unique<BindableRNodeInput<Texture>>("brdf", this, iBRDFTexBindable) );
			addInput( std::make_unique<BindableRNodeInput<Texture>>("shadow", this, iShadowTexBindable) );
			addInput( std::make_unique<BindableRNodeInput<Texture>>("position", this, iPositionTexBindable) );
			addInput( std::make_unique<BindableRNodeInput<Texture>>("normal", this, iNormalTexBindable) );
			addInput( std::make_unique<BindableRNodeInput<Texture>>("albedo", this, iAlbedoTexBindable) );
			addInput( std::make_unique<BindableRNodeInput<Texture>>("material", this, iMaterialTexBindable) );
			addInput( std::make_unique<BindableRNodeInput<Texture>>("emissive", this, iEmissiveTexBindable) );

			auto program = repository.find<std::string, Program>("programDeferredLighting");
			if (!program) {
				program = TechniqueLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentDeferredLighting.glsl");
				repository.add("programDeferredLighting"s, program);
			}

			mViewPosition = std::make_shared<UniformVariableValue<glm::vec3>>("uViewPosition", *program, glm::vec3(0.0f));
			mNumLights = std::make_shared<UniformVariableValue<unsigned int>>("uNumLights", *program, 0);
			mLightsBuffer = std::make_shared<UniformBuffer>();
			utils::FixedVector<ShaderLightSource, kMaxLights> lightsBufferData(kMaxLights);
			mLightsBuffer->resizeAndCopy(lightsBufferData.data(), lightsBufferData.size());
			mShadowLightIndex = std::make_shared<UniformVariableValue<unsigned int>>("uShadowLightIndex", *program, kMaxLights);
			mShadowViewMatrix = std::make_shared<UniformVariableValue<glm::mat4>>("uShadowViewMatrix", *program, glm::mat4(1.0f));
			mShadowProjectionMatrix = std::make_shared<UniformVariableValue<glm::mat4>>("uShadowProjectionMatrix", *program, glm::mat4(1.0f));

			addBindable(program);
			addBindable(std::make_shared<SetDepthMask>(false));
			addBindable(std::make_shared<UniformVariableValue<glm::mat4>>("uModelMatrix", *program, glm::mat4(1.0f)));
			addBindable(std::make_shared<UniformVariableValue<glm::mat4>>("uViewMatrix", *program, glm::mat4(1.0f)));
			addBindable(std::make_shared<UniformVariableValue<glm::mat4>>("uProjectionMatrix", *program, glm::mat4(1.0f)));
			addBindable(mShadowViewMatrix);
			addBindable(mShadowProjectionMatrix);
			addBindable(mViewPosition);
			addBindable(std::make_shared<UniformVariableValue<int>>("uIrradianceMap", *program, EnvironmentTexUnits::kIrradianceMap));
			addBindable(std::make_shared<UniformVariableValue<int>>("uPrefilterMap", *program, EnvironmentTexUnits::kPrefilterMap));
			addBindable(std::make_shared<UniformVariableValue<int>>("uBRDFMap", *program, EnvironmentTexUnits::kBRDFMap));
			addBindable(std::make_shared<UniformVariableValue<int>>("uShadowMap", *program, EnvironmentTexUnits::kShadowMap));
			addBindable(std::make_shared<UniformVariableValue<int>>("uPosition", *program, GBufferTexUnits::kPosition));
			addBindable(std::make_shared<UniformVariableValue<int>>("uNormal", *program, GBufferTexUnits::kNormal));
			addBindable(std::make_shared<UniformVariableValue<int>>("uAlbedo", *program, GBufferTexUnits::kAlbedo));
			addBindable(std::make_shared<UniformVariableValue<int>>("uMaterial", *program, GBufferTexUnits::kMaterial));
			addBindable(std::make_shared<UniformVariableValue<int>>("uEmissive", *program, GBufferTexUnits::kEmissive));
			addBindable(mLightsBuffer);
			addBindable(mNumLights);
			addBindable(mShadowLightIndex);
			addBindable(std::make_shared<UniformBlock>("LightsBlock", *program));
		}

		void setViewPosition(const glm::vec3& position)
		{
			mViewPosition->setValue(position);
		};

		void setLights(const ShaderLightSource* lightSources, unsigned int lightSourceCount)
		{
			mLightsBuffer->copy(lightSources, lightSourceCount);
			mNumLights->setValue(lightSourceCount);
		};

		void setShadow(unsigned int shadowLightIndex, const glm::mat4& shadowViewMatrix, const glm::mat4& shadowProjectionMatrix)
		{
			mShadowLightIndex->setValue(shadowLightIndex);
			mShadowViewMatrix->setValue(shadowViewMatrix);
			mShadowProjectionMatrix->setValue(shadowProjectionMatrix);
		};

		virtual void execute() override
		{
			bind();
			mPlane->draw();
		};
	};


	class AppRenderer::ShadowRenderer : public Renderer3D
	{
	private:
		std::size_t mShadowResolution;
	public:
		ShadowRenderer(const std::string& name) : Renderer3D(name), mShadowResolution(0)
		{
			addBindable(std::make_shared<SetOperation>(graphics::Operation::DepthTest));
		};

		ShadowRenderer& setShadowResolution(std::size_t shadowResolution)
		{
			mShadowResolution = shadowResolution;
			return *this;
		};

		virtual void execute() override
		{
			int lastX, lastY;
			std::size_t lastWidth, lastHeight;
			se::graphics::GraphicsOperations::getViewport(lastX, lastY, lastWidth, lastHeight);

			GraphicsOperations::setViewport(0, 0, mShadowResolution, mShadowResolution);
			GraphicsOperations::setCullingMode(FaceMode::Front);
			Renderer3D::execute();
			GraphicsOperations::setCullingMode(FaceMode::Back);

			GraphicsOperations::setViewport(lastX, lastY, lastWidth, lastHeight);
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
			addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("target", this, iTargetBindable) );

			addInput( std::make_unique<BindableRNodeInput<Texture>>("color0", this, addBindable()) );
			addInput( std::make_unique<BindableRNodeInput<Texture>>("color1", this, addBindable()) );

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

		addResources(width, height);
		addNodes(width, height);
		mApplication.getExternalTools().graphicsEngine->getRenderGraph().prepareGraph();
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

		unsigned int i = 0, iShadowLight = DeferredLightRenderer::kMaxLights;
		std::array<DeferredLightRenderer::ShaderLightSource, DeferredLightRenderer::kMaxLights> uBaseLights;
		mEntityDatabase.iterateComponents<TransformsComponent, LightComponent>(
			[&](Entity entity, TransformsComponent* transforms, LightComponent* light) {
				if (i < DeferredLightRenderer::kMaxLights) {
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

		mDeferredLightRenderer->setLights(uBaseLights.data(), i);

		auto [cameraTransforms] = mEntityDatabase.getComponents<TransformsComponent>(mCameraEntity);
		if (cameraTransforms) {
			mDeferredLightRenderer->setViewPosition(cameraTransforms->position);
		}

		auto [shadowTransforms] = mEntityDatabase.getComponents<TransformsComponent>(mShadowEntity);
		if (shadowTransforms) {
			mShadowCamera.setPosition(shadowTransforms->position);
			mShadowCamera.setTarget(shadowTransforms->position + glm::vec3(0.0f, 0.0f, 1.0f) * shadowTransforms->orientation);
			mShadowCamera.setUp({ 0.0f, 1.0f, 0.0f });
		}
		mDeferredLightRenderer->setShadow(iShadowLight, mShadowCamera.getViewMatrix(), mShadowCamera.getProjectionMatrix());

		SOMBRA_INFO_LOG << "Update end";
	}


	void AppRenderer::render()
	{
		SOMBRA_INFO_LOG << "Render start";
		mApplication.getExternalTools().graphicsEngine->render();
		SOMBRA_INFO_LOG << "Render end";
	}

// Private functions
	bool AppRenderer::addResources(std::size_t width, std::size_t height)
	{
		auto& graphicsEngine = *mApplication.getExternalTools().graphicsEngine;
		auto resources = dynamic_cast<BindableRenderNode*>(graphicsEngine.getRenderGraph().getNode("resources"));

		RawMesh planeRawMesh;
		planeRawMesh.positions = { {-1.0f,-1.0f, 0.0f }, { 1.0f,-1.0f, 0.0f }, {-1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f } };
		planeRawMesh.faceIndices = { 0, 1, 2, 1, 3, 2, };
		auto planeMesh = std::make_unique<Mesh>(MeshLoader::createGraphicsMesh(planeRawMesh));
		mPlaneRenderable = std::make_shared<RenderableMesh>(std::move(planeMesh));

		mIrradianceTextureResource = resources->addBindable();
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("irradianceTexture", resources, mIrradianceTextureResource) )) {
			return false;
		}

		mPrefilterTextureResource = resources->addBindable();
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("prefilterTexture", resources, mPrefilterTextureResource) )) {
			return false;
		}

		auto brdfTexture = TextureUtils::precomputeBRDF(512);
		brdfTexture->setTextureUnit(EnvironmentTexUnits::kBRDFMap);
		auto iBRDFTextureResource = resources->addBindable(brdfTexture);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("brdfTexture", resources, iBRDFTextureResource) )) {
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


	bool AppRenderer::addNodes(std::size_t width, std::size_t height)
	{
		auto& graphicsEngine = *mApplication.getExternalTools().graphicsEngine;
		auto& renderGraph = graphicsEngine.getRenderGraph();

		if (!addShadowRenderer(renderGraph)
			|| !addDeferredRenderer(renderGraph, width, height)
			|| !addForwardRenderer(renderGraph)
		) {
			return false;
		}

		auto clearMask = FrameBufferMask::Mask().set(FrameBufferMask::kColor).set(FrameBufferMask::kDepth);
		auto defaultFBClear = std::make_unique<FBClearNode>("defaultFBClear", clearMask);
		auto deferredFBClear = std::make_unique<FBClearNode>("deferredFBClear", clearMask);

		// Node used for combining the shadow map and the forward and deferred renderers
		auto texUnitNodeShadow = std::make_unique<TextureUnitNode>("texUnitNodeShadow", EnvironmentTexUnits::kShadowMap);

		// Node used for combining the zBuffer of the deferred and forward renderers
		auto zBufferCopy = std::make_unique<FBCopyNode>("zBufferCopy", FrameBufferMask::Mask().set(FrameBufferMask::kDepth));
		zBufferCopy->setDimensions1(0, 0, width, height).setDimensions2(0, 0, width, height);

		// Nodes used for blurring the bright colors (bloom)
		auto hBlurNode = std::make_unique<GaussianBlurNode>("hBlurNode", mApplication.getRepository(), mPlaneRenderable, width, height, true);
		auto vBlurNode = std::make_unique<GaussianBlurNode>("vBlurNode", mApplication.getRepository(), mPlaneRenderable, width, height, false);
		auto hBlurTexUnitNode = std::make_unique<TextureUnitNode>("hBlurTexUnitNode", GaussianBlurNode::kColorTextureUnit);
		auto vBlurTexUnitNode = std::make_unique<TextureUnitNode>("vBlurTexUnitNode", GaussianBlurNode::kColorTextureUnit);

		// Node used for combining the bloom and color
		auto combine0TexUnitNode = std::make_unique<TextureUnitNode>("combine0TexUnitNode", CombineNode::kColor0);
		auto combine1TexUnitNode = std::make_unique<TextureUnitNode>("combine1TexUnitNode", CombineNode::kColor1);
		auto combineBloomNode = std::make_unique<CombineNode>("combineBloomNode", mApplication.getRepository(), mPlaneRenderable);

		// Node used for drawing 2D renderables
		auto renderer2D = std::make_unique<Renderer2D>("renderer2D");

		// Link the render graph nodes
		auto resources = renderGraph.getNode("resources"),
			shadowRenderer = renderGraph.getNode("shadowRenderer"),
			gBufferRenderer = renderGraph.getNode("gBufferRenderer"),
			deferredLightRenderer = renderGraph.getNode("deferredLightRenderer"),
			forwardRenderer = renderGraph.getNode("forwardRenderer");

		return defaultFBClear->findInput("input")->connect( resources->findOutput("defaultFB") )
			&& deferredFBClear->findInput("input")->connect( resources->findOutput("deferredBuffer") )
			&& texUnitNodeShadow->findInput("input")->connect( shadowRenderer->findOutput("shadow") )
			&& deferredLightRenderer->findInput("target")->connect( deferredFBClear->findOutput("output") )
			&& deferredLightRenderer->findInput("irradiance")->connect( resources->findOutput("irradianceTexture") )
			&& deferredLightRenderer->findInput("prefilter")->connect( resources->findOutput("prefilterTexture") )
			&& deferredLightRenderer->findInput("brdf")->connect( resources->findOutput("brdfTexture") )
			&& deferredLightRenderer->findInput("shadow")->connect( texUnitNodeShadow->findOutput("output") )
			&& zBufferCopy->findInput("input1")->connect( deferredLightRenderer->findOutput("target") )
			&& zBufferCopy->findInput("input2")->connect( gBufferRenderer->findOutput("target") )
			&& forwardRenderer->findInput("target")->connect( zBufferCopy->findOutput("output") )
			&& forwardRenderer->findInput("irradiance")->connect( resources->findOutput("irradianceTexture") )
			&& forwardRenderer->findInput("prefilter")->connect( resources->findOutput("prefilterTexture") )
			&& forwardRenderer->findInput("brdf")->connect( resources->findOutput("brdfTexture") )
			&& forwardRenderer->findInput("shadow")->connect( texUnitNodeShadow->findOutput("output") )
			&& forwardRenderer->findInput("color")->connect( resources->findOutput("colorTexture") )
			&& forwardRenderer->findInput("bright")->connect( resources->findOutput("brightTexture") )
			&& hBlurTexUnitNode->findInput("input")->connect( forwardRenderer->findOutput("bright") )
			&& hBlurNode->findInput("input")->connect( hBlurTexUnitNode->findOutput("output") )
			&& vBlurTexUnitNode->findInput("input")->connect( hBlurNode->findOutput("output") )
			&& vBlurNode->findInput("input")->connect( vBlurTexUnitNode->findOutput("output") )
			&& combine0TexUnitNode->findInput("input")->connect( forwardRenderer->findOutput("color") )
			&& combine1TexUnitNode->findInput("input")->connect( vBlurNode->findOutput("output") )
			&& combineBloomNode->findInput("target")->connect( defaultFBClear->findOutput("output") )
			&& combineBloomNode->findInput("color0")->connect( combine0TexUnitNode->findOutput("output") )
			&& combineBloomNode->findInput("color1")->connect( combine1TexUnitNode->findOutput("output") )
			&& renderer2D->findInput("target")->connect( combineBloomNode->findOutput("target") )
			&& renderGraph.addNode( std::move(defaultFBClear) )
			&& renderGraph.addNode( std::move(deferredFBClear) )
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


	bool AppRenderer::addDeferredRenderer(graphics::RenderGraph& renderGraph, std::size_t width, std::size_t height)
	{
		auto resources = dynamic_cast<BindableRenderNode*>(renderGraph.getNode("resources"));

		// Create the gBuffer FB
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

		// Create the gBuffer FB clear node
		auto clearMask = FrameBufferMask::Mask().set(FrameBufferMask::kColor).set(FrameBufferMask::kDepth);
		auto gFBClear = std::make_unique<FBClearNode>("gFBClear", clearMask);

		// Create the gBufferRenderer
		auto gBufferRenderer = std::make_unique<Renderer3D>("gBufferRenderer");

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

		// Create the deferredLightRenderer
		auto deferredLightRenderer = std::make_unique<DeferredLightRenderer>("deferredLightRenderer", mApplication.getRepository(), mPlaneRenderable);
		mDeferredLightRenderer = deferredLightRenderer.get();

		auto texUnitNodePosition = std::make_unique<TextureUnitNode>("texUnitNodePosition", DeferredLightRenderer::GBufferTexUnits::kPosition);
		auto texUnitNodeNormal = std::make_unique<TextureUnitNode>("texUnitNodeNormal", DeferredLightRenderer::GBufferTexUnits::kNormal);
		auto texUnitNodeAlbedo = std::make_unique<TextureUnitNode>("texUnitNodeAlbedo", DeferredLightRenderer::GBufferTexUnits::kAlbedo);
		auto texUnitNodeMaterial = std::make_unique<TextureUnitNode>("texUnitNodeMaterial", DeferredLightRenderer::GBufferTexUnits::kMaterial);
		auto texUnitNodeEmissive = std::make_unique<TextureUnitNode>("texUnitNodeEmissive", DeferredLightRenderer::GBufferTexUnits::kEmissive);

		// Add the nodes and their connections
		return gFBClear->findInput("input")->connect( resources->findOutput("gBuffer") )
			&& gBufferRenderer->findInput("target")->connect( gFBClear->findOutput("output") )
			&& texUnitNodePosition->findInput("input")->connect( gBufferRenderer->findOutput("position") )
			&& texUnitNodeNormal->findInput("input")->connect( gBufferRenderer->findOutput("normal") )
			&& texUnitNodeAlbedo->findInput("input")->connect( gBufferRenderer->findOutput("albedo") )
			&& texUnitNodeMaterial->findInput("input")->connect( gBufferRenderer->findOutput("material") )
			&& texUnitNodeEmissive->findInput("input")->connect( gBufferRenderer->findOutput("emissive") )
			&& deferredLightRenderer->findInput("position")->connect( texUnitNodePosition->findOutput("output") )
			&& deferredLightRenderer->findInput("normal")->connect( texUnitNodeNormal->findOutput("output") )
			&& deferredLightRenderer->findInput("albedo")->connect( texUnitNodeAlbedo->findOutput("output") )
			&& deferredLightRenderer->findInput("material")->connect( texUnitNodeMaterial->findOutput("output") )
			&& deferredLightRenderer->findInput("emissive")->connect( texUnitNodeEmissive->findOutput("output") )
			&& renderGraph.addNode( std::move(gFBClear) )
			&& renderGraph.addNode( std::move(gBufferRenderer) )
			&& renderGraph.addNode( std::move(texUnitNodePosition) )
			&& renderGraph.addNode( std::move(texUnitNodeNormal) )
			&& renderGraph.addNode( std::move(texUnitNodeAlbedo) )
			&& renderGraph.addNode( std::move(texUnitNodeMaterial) )
			&& renderGraph.addNode( std::move(texUnitNodeEmissive) )
			&& renderGraph.addNode( std::move(deferredLightRenderer) );
	}


	bool AppRenderer::addForwardRenderer(graphics::RenderGraph& renderGraph)
	{
		auto forwardRenderer = std::make_unique<Renderer3D>("forwardRenderer");
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


	bool AppRenderer::addShadowRenderer(graphics::RenderGraph& renderGraph)
	{
		auto resources = dynamic_cast<BindableRenderNode*>(renderGraph.getNode("resources"));

		// Create the shadow FB
		auto shadowTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		shadowTexture->setImage(nullptr, TypeId::Float, ColorFormat::Depth, ColorFormat::Depth, mShadowData.resolution, mShadowData.resolution)
			.setWrapping(TextureWrap::ClampToBorder, TextureWrap::ClampToBorder)
			.setBorderColor(1.0f, 1.0f, 1.0f, 1.0f)
			.setFiltering(TextureFilter::Nearest, TextureFilter::Nearest);

		auto shadowBuffer = std::make_shared<FrameBuffer>();
		shadowBuffer->setColorBuffer(false)
			.attach(*shadowTexture, FrameBufferAttachment::kDepth);
		auto iShadowBufferResource = resources->addBindable(shadowBuffer);
		if (!resources->addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("shadowBuffer", resources, iShadowBufferResource) )) {
			return false;
		}

		// Create the shadow FB clear node
		auto shadowFBClear = std::make_unique<FBClearNode>("shadowFBClear", FrameBufferMask::Mask().set(FrameBufferMask::kDepth));

		// Create the shadow Renderer
		auto shadowRenderer = std::make_unique<ShadowRenderer>("shadowRenderer");
		shadowRenderer->setShadowResolution(mShadowData.resolution);

		auto iShadowTexBindable = shadowRenderer->addBindable(shadowTexture, false);
		shadowRenderer->addOutput( std::make_unique<BindableRNodeOutput<Texture>>("shadow", shadowRenderer.get(), iShadowTexBindable) );

		// Add the nodes and their connections
		return shadowFBClear->findInput("input")->connect( resources->findOutput("shadowBuffer") )
			&& shadowRenderer->findInput("target")->connect( shadowFBClear->findOutput("output") )
			&& renderGraph.addNode( std::move(shadowRenderer) )
			&& renderGraph.addNode( std::move(shadowFBClear) );
	}


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
		auto width = static_cast<std::size_t>(event.getWidth());
		auto height = static_cast<std::size_t>(event.getHeight());
		GraphicsOperations::setViewport(0, 0, width, height);
	}

}
