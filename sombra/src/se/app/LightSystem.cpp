#include <glm/gtx/compatibility.hpp>
#include "se/utils/Log.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/graphics/Renderer.h"
#include "se/app/io/MeshLoader.h"
#include "se/app/io/ShaderLoader.h"
#include "se/app/LightSystem.h"
#include "se/app/Application.h"
#include "se/app/Repository.h"
#include "se/app/MeshComponent.h"
#include "se/app/TerrainComponent.h"
#include "se/app/TransformsComponent.h"
#include "se/app/CameraComponent.h"
#include "graphics/DeferredLightSubGraph.h"

namespace se::app {

	static const glm::quat kCubeMapOrientations[] = {
		glm::conjugate(glm::quat_cast( glm::lookAt(glm::vec3(0.0f), { 1.0f, 0.0f, 0.0f }, { 0.0f,-1.0f, 0.0f }) )),
		glm::conjugate(glm::quat_cast( glm::lookAt(glm::vec3(0.0f), {-1.0f, 0.0f, 0.0f }, { 0.0f,-1.0f, 0.0f }) )),
		glm::conjugate(glm::quat_cast( glm::lookAt(glm::vec3(0.0f), { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }) )),
		glm::conjugate(glm::quat_cast( glm::lookAt(glm::vec3(0.0f), { 0.0f,-1.0f, 0.0f }, { 0.0f, 0.0f,-1.0f }) )),
		glm::conjugate(glm::quat_cast( glm::lookAt(glm::vec3(0.0f), { 0.0f, 0.0f, 1.0f }, { 0.0f,-1.0f, 0.0f }) )),
		glm::conjugate(glm::quat_cast( glm::lookAt(glm::vec3(0.0f), { 0.0f, 0.0f,-1.0f }, { 0.0f,-1.0f, 0.0f }) )),
	};


	struct LightSystem::LightVolumeData
	{
		using PassSPtr = std::shared_ptr<graphics::Pass>;
		using TechniqueSPtr = std::shared_ptr<graphics::Technique>;
		using RenderableShaderStepResource = Repository::ResourceRef<RenderableShaderStep>;
		using RenderableShaderResource = Repository::ResourceRef<RenderableShader>;

		static constexpr std::size_t kDL = 0;
		static constexpr std::size_t kDLCSM = 1;
		static constexpr std::size_t kDLPLShadows = 2;
		static constexpr std::size_t kNumDL = 3;

		MeshRef pointLight, spotLight, directionalLight;
		PassSPtr passLight;
		TechniqueSPtr techniqueLight;

		ProgramRef programDeferredStencil;
		RenderableShaderStepResource stepDeferredStencil;

		ProgramRef programDeferredLighting[kNumDL];
		RenderableShaderStepResource stepDeferredLighting[kNumDL];
		RenderableShaderResource shaderDeferredLighting[kNumDL];
		UniformVVRef<glm::vec3> cameraPosition[kNumDL];
	};


	LightSystem::LightSystem(Application& application, float shadowSplitLogFactor) :
		ISystem(application.getEntityDatabase()), mApplication(application), mShadowSplitLogFactor(shadowSplitLogFactor)
	{
		mApplication.getEventManager()
			.subscribe(this, Topic::Camera)
			.subscribe(this, Topic::RMesh)
			.subscribe(this, Topic::LightSource)
			.subscribe(this, Topic::RShader)
			.subscribe(this, Topic::Shader);
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask()
			.set<LightComponent>()
			.set<MeshComponent>()
			.set<TerrainComponent>()
		);

		Result result;
		auto& context = mApplication.getExternalTools().graphicsEngine->getContext();
		auto& renderGraph = mApplication.getExternalTools().graphicsEngine->getRenderGraph();
		mLightVolumeData = std::make_unique<LightVolumeData>();

		// Meshes
		{
			RawMesh rawMesh = MeshLoader::createSphereMesh("pointLight", 16, 10, 0.5f);
			rawMesh.texCoords.clear();
			auto bounds = MeshLoader::calculateBounds(rawMesh);
			mLightVolumeData->pointLight = MeshLoader::createGraphicsMesh(context, rawMesh);
			mLightVolumeData->pointLight.edit([=](graphics::Mesh& m) { m.setBounds(bounds.first, bounds.second); });
		}

		{
			RawMesh rawMesh = MeshLoader::createConeMesh("spotLight", 16, 0.5f, 1.0f);
			rawMesh.texCoords.clear();
			auto bounds = MeshLoader::calculateBounds(rawMesh);
			mLightVolumeData->spotLight = MeshLoader::createGraphicsMesh(context, rawMesh);
			mLightVolumeData->spotLight.edit([=](graphics::Mesh& m) { m.setBounds(bounds.first, bounds.second); });
		}

		{
			RawMesh rawMesh = MeshLoader::createBoxMesh("directionalLight", glm::vec3(1.0f));
			rawMesh.texCoords.clear();
			auto bounds = MeshLoader::calculateBounds(rawMesh);
			mLightVolumeData->directionalLight = MeshLoader::createGraphicsMesh(context, rawMesh);
			mLightVolumeData->directionalLight.edit([=](graphics::Mesh& m) { m.setBounds(bounds.first, bounds.second); });
		}

		// Programs
		result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, nullptr, context, mLightVolumeData->programDeferredStencil);
		if (!result) {
			SOMBRA_ERROR_LOG << result.description();
			return;
		}

		result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentDeferredLighting.glsl", context, mLightVolumeData->programDeferredLighting[LightVolumeData::kDL]);
		if (!result) {
			SOMBRA_ERROR_LOG << result.description();
			return;
		}

		result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentDeferredLightingCSM.glsl", context, mLightVolumeData->programDeferredLighting[LightVolumeData::kDLCSM]);
		if (!result) {
			SOMBRA_ERROR_LOG << result.description();
			return;
		}

		result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentDeferredLightingPLShadows.glsl", context, mLightVolumeData->programDeferredLighting[LightVolumeData::kDLPLShadows]);
		if (!result) {
			SOMBRA_ERROR_LOG << result.description();
			return;
		}

		// Passes
		auto deferredLightSubGraph = dynamic_cast<DeferredLightSubGraph*>( renderGraph.getNode("deferredLightSubGraph") );
		mShadowRenderSubGraph = deferredLightSubGraph->getShadowRenderSubGraph();
		mLightVolumeData->passLight = std::make_shared<graphics::Pass>(*deferredLightSubGraph);

		// Techniques
		mLightVolumeData->techniqueLight = std::make_shared<graphics::Technique>();
		mLightVolumeData->techniqueLight->addPass(mLightVolumeData->passLight);

		// RenderableShaderSteps
		mLightVolumeData->stepDeferredStencil = mApplication.getRepository().findByName<RenderableShaderStep>("stepDeferredStencil");
		if (!mLightVolumeData->stepDeferredStencil) {
			mLightVolumeData->stepDeferredStencil = mApplication.getRepository().insert(std::make_shared<RenderableShaderStep>(*deferredLightSubGraph->getStencilRenderer()), "stepDeferredStencil");
			mLightVolumeData->stepDeferredStencil->addBindable(mLightVolumeData->programDeferredStencil);
		}

		mLightVolumeData->stepDeferredLighting[LightVolumeData::kDL] = mApplication.getRepository().findByName<RenderableShaderStep>("stepDeferredLighting");
		if (!mLightVolumeData->stepDeferredLighting[LightVolumeData::kDL]) {
			auto program = mLightVolumeData->programDeferredLighting[LightVolumeData::kDL];
			mLightVolumeData->cameraPosition[LightVolumeData::kDL] = context.create<graphics::UniformVariableValue<glm::vec3>>("uViewPosition")
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); });

			mLightVolumeData->stepDeferredLighting[LightVolumeData::kDL] = mApplication.getRepository().insert(std::make_shared<RenderableShaderStep>(*deferredLightSubGraph->getColorRenderer()), "stepDeferredLighting");
			mLightVolumeData->stepDeferredLighting[LightVolumeData::kDL]->addBindable(program)
				.addBindable(
					context.create<graphics::UniformVariableValue<int>>("uPosition", DeferredLightSubGraph::TexUnits::kPosition)
						.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
				)
				.addBindable(
					context.create<graphics::UniformVariableValue<int>>("uNormal", DeferredLightSubGraph::TexUnits::kNormal)
						.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
				)
				.addBindable(
					context.create<graphics::UniformVariableValue<int>>("uAlbedo", DeferredLightSubGraph::TexUnits::kAlbedo)
						.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
				)
				.addBindable(
					context.create<graphics::UniformVariableValue<int>>("uMaterial", DeferredLightSubGraph::TexUnits::kMaterial)
						.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
				)
				.addBindable(mLightVolumeData->cameraPosition[LightVolumeData::kDL]);
		}

		mLightVolumeData->stepDeferredLighting[LightVolumeData::kDLCSM] = mApplication.getRepository().findByName<RenderableShaderStep>("stepDeferredLightingCSM");
		if (!mLightVolumeData->stepDeferredLighting[LightVolumeData::kDLCSM]) {
			auto program = mLightVolumeData->programDeferredLighting[LightVolumeData::kDLCSM];
			mLightVolumeData->cameraPosition[LightVolumeData::kDLCSM] = context.create<graphics::UniformVariableValue<glm::vec3>>("uViewPosition")
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); });

			mLightVolumeData->stepDeferredLighting[LightVolumeData::kDLCSM] = mApplication.getRepository().insert(std::make_shared<RenderableShaderStep>(*deferredLightSubGraph->getColorRenderer()), "stepDeferredLightingCSM");
			mLightVolumeData->stepDeferredLighting[LightVolumeData::kDLCSM]->addBindable(program)
				.addBindable(
					context.create<graphics::UniformVariableValue<int>>("uPosition", DeferredLightSubGraph::TexUnits::kPosition)
						.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
				)
				.addBindable(
					context.create<graphics::UniformVariableValue<int>>("uNormal", DeferredLightSubGraph::TexUnits::kNormal)
						.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
				)
				.addBindable(
					context.create<graphics::UniformVariableValue<int>>("uAlbedo", DeferredLightSubGraph::TexUnits::kAlbedo)
						.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
				)
				.addBindable(
					context.create<graphics::UniformVariableValue<int>>("uMaterial", DeferredLightSubGraph::TexUnits::kMaterial)
						.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
				)
				.addBindable(
					context.create<graphics::UniformVariableValue<int>>("uShadow", DeferredLightSubGraph::TexUnits::kShadow)
						.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
				)
				.addBindable(mLightVolumeData->cameraPosition[LightVolumeData::kDLCSM]);
		}

		mLightVolumeData->stepDeferredLighting[LightVolumeData::kDLPLShadows] = mApplication.getRepository().findByName<RenderableShaderStep>("stepDeferredLightingPLShadows");
		if (!mLightVolumeData->stepDeferredLighting[LightVolumeData::kDLPLShadows]) {
			auto program = mLightVolumeData->programDeferredLighting[LightVolumeData::kDLPLShadows];
			mLightVolumeData->cameraPosition[LightVolumeData::kDLPLShadows] = context.create<graphics::UniformVariableValue<glm::vec3>>("uViewPosition")
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); });

			mLightVolumeData->stepDeferredLighting[LightVolumeData::kDLPLShadows] = mApplication.getRepository().insert(std::make_shared<RenderableShaderStep>(*deferredLightSubGraph->getColorRenderer()), "stepDeferredLightingPLShadows");
			mLightVolumeData->stepDeferredLighting[LightVolumeData::kDLPLShadows]->addBindable(program)
				.addBindable(
					context.create<graphics::UniformVariableValue<int>>("uPosition", DeferredLightSubGraph::TexUnits::kPosition)
						.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
				)
				.addBindable(
					context.create<graphics::UniformVariableValue<int>>("uNormal", DeferredLightSubGraph::TexUnits::kNormal)
						.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
				)
				.addBindable(
					context.create<graphics::UniformVariableValue<int>>("uAlbedo", DeferredLightSubGraph::TexUnits::kAlbedo)
						.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
				)
				.addBindable(
					context.create<graphics::UniformVariableValue<int>>("uMaterial", DeferredLightSubGraph::TexUnits::kMaterial)
						.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
				)
				.addBindable(
					context.create<graphics::UniformVariableValue<int>>("uShadow", DeferredLightSubGraph::TexUnits::kShadow)
						.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
				)
				.addBindable(mLightVolumeData->cameraPosition[LightVolumeData::kDLPLShadows]);
		}

		// RenderableShaders
		mLightVolumeData->shaderDeferredLighting[LightVolumeData::kDL] = mApplication.getRepository().findByName<RenderableShader>("shaderDeferredLight");
		if (!mLightVolumeData->shaderDeferredLighting[LightVolumeData::kDL]) {
			mLightVolumeData->shaderDeferredLighting[LightVolumeData::kDL] = mApplication.getRepository().insert(std::make_shared<RenderableShader>(mApplication.getEventManager()), "shaderDeferredLight");
			mLightVolumeData->shaderDeferredLighting[LightVolumeData::kDL]->addStep(mLightVolumeData->stepDeferredStencil)
				.addStep(mLightVolumeData->stepDeferredLighting[LightVolumeData::kDL]);
		}

		mLightVolumeData->shaderDeferredLighting[LightVolumeData::kDLCSM] = mApplication.getRepository().findByName<RenderableShader>("shaderDeferredLightCSM");
		if (!mLightVolumeData->shaderDeferredLighting[LightVolumeData::kDLCSM]) {
			mLightVolumeData->shaderDeferredLighting[LightVolumeData::kDLCSM] = mApplication.getRepository().insert(std::make_shared<RenderableShader>(mApplication.getEventManager()), "shaderDeferredLightCSM");
			mLightVolumeData->shaderDeferredLighting[LightVolumeData::kDLCSM]->addStep(mLightVolumeData->stepDeferredStencil)
				.addStep(mLightVolumeData->stepDeferredLighting[LightVolumeData::kDLCSM]);
		}

		mLightVolumeData->shaderDeferredLighting[LightVolumeData::kDLPLShadows] = mApplication.getRepository().findByName<RenderableShader>("shaderDeferredLightPLShadows");
		if (!mLightVolumeData->shaderDeferredLighting[LightVolumeData::kDLPLShadows]) {
			mLightVolumeData->shaderDeferredLighting[LightVolumeData::kDLPLShadows] = mApplication.getRepository().insert(std::make_shared<RenderableShader>(mApplication.getEventManager()), "shaderDeferredLightPLShadows");
			mLightVolumeData->shaderDeferredLighting[LightVolumeData::kDLPLShadows]->addStep(mLightVolumeData->stepDeferredStencil)
				.addStep(mLightVolumeData->stepDeferredLighting[LightVolumeData::kDLPLShadows]);
		}
	}


	LightSystem::~LightSystem()
	{
		mEntityDatabase.removeSystem(this);
		mApplication.getEventManager()
			.unsubscribe(this, Topic::Shader)
			.unsubscribe(this, Topic::RShader)
			.unsubscribe(this, Topic::RMesh)
			.unsubscribe(this, Topic::LightSource)
			.unsubscribe(this, Topic::RendererResolution)
			.unsubscribe(this, Topic::Camera);
	}


	bool LightSystem::notify(const IEvent& event)
	{
		return tryCall(&LightSystem::onCameraEvent, event)
			|| tryCall(&LightSystem::onRMeshEvent, event)
			|| tryCall(&LightSystem::onLightSourceEvent, event)
			|| tryCall(&LightSystem::onRenderableShaderEvent, event)
			|| tryCall(&LightSystem::onShaderEvent, event);
	}


	void LightSystem::onNewComponent(Entity entity, const EntityDatabase::ComponentMask& mask, EntityDatabase::Query& query)
	{
		tryCallC(&LightSystem::onNewLight, entity, mask, query);
		tryCallC(&LightSystem::onNewMesh, entity, mask, query);
		tryCallC(&LightSystem::onNewTerrain, entity, mask, query);
	}


	void LightSystem::onRemoveComponent(Entity entity, const EntityDatabase::ComponentMask& mask, EntityDatabase::Query& query)
	{
		tryCallC(&LightSystem::onRemoveLight, entity, mask, query);
		tryCallC(&LightSystem::onRemoveMesh, entity, mask, query);
		tryCallC(&LightSystem::onRemoveTerrain, entity, mask, query);
	}


	void LightSystem::update(float, float)
	{
		SOMBRA_DEBUG_LOG << "Updating the Lights";

		CameraComponent shadowsCamera;
		glm::mat4 shadowViewMatrix, shadowProjectionMatrix, camViewMatrix(1.0f);
		utils::FixedVector<glm::mat4, RenderableLight::kMaxShadowMaps> shadowVPMatrices;
		float size, zNear, zFar, camFOVY(0.0f), camAspectRatio(1.0f), camZNear(-1.0f), camZFar(1.0f);
		std::size_t resolution, numCascades;

		mEntityDatabase.executeQuery([&](EntityDatabase::Query& query) {
			std::scoped_lock lock(mMutex);
			SOMBRA_DEBUG_LOG << "Checking if the camera was updated";

			auto [transforms, camera] = query.getComponents<TransformsComponent, CameraComponent>(mCameraEntity, true);
			if (transforms && camera) {
				for (std::size_t i = 0; i < LightVolumeData::kNumDL; ++i) {
					mLightVolumeData->cameraPosition[i].edit([p = transforms->position](auto& uniform) { uniform.setValue(p); });
				}

				if (!camera->hasOrthographicProjection()) {
					camera->getPerspectiveParams(camFOVY, camAspectRatio, camZNear, camZFar);
					camViewMatrix = camera->getViewMatrix();
				}
			}
		});

		mEntityDatabase.executeQuery([&](EntityDatabase::Query& query) {
			query.iterateEntityComponents<TransformsComponent, LightComponent>(
				[&](Entity entity, TransformsComponent* transforms, LightComponent* light) {
					std::scoped_lock lock(mMutex);
					auto itUniforms = mEntityUniforms.find(entity);
					if (itUniforms == mEntityUniforms.end()) { return; }

					glm::mat4	translation = glm::translate(glm::mat4(1.0f), transforms->position),
								rotation = glm::mat4_cast(transforms->orientation),
								scale = glm::mat4(1.0f);

					itUniforms->second.type.edit([t = light->getSource()->getType()](auto& uniform) { uniform.setValue(static_cast<int>(t)); });
					itUniforms->second.color.edit([c = light->getSource()->getColor()](auto& uniform) { uniform.setValue(c); });
					itUniforms->second.intensity.edit([i = light->getSource()->getIntensity()](auto& uniform) { uniform.setValue(i); });
					itUniforms->second.range.edit([r = light->getSource()->getRange()](auto& uniform) { uniform.setValue(r); });

					switch (light->getSource()->getType()) {
						case LightSource::Type::Point: {
							scale = glm::scale(glm::mat4(1.0f), glm::vec3(light->getSource()->getRange()));

							if (light->getRenderable().castsShadows()) {
								light->getSource()->getShadows(resolution, zNear, zFar, size, numCascades);

								shadowsCamera.setPosition(transforms->position);
								shadowsCamera.setPerspectiveProjection(glm::half_pi<float>(), 1.0f, zNear, zFar);
								shadowProjectionMatrix = shadowsCamera.getProjectionMatrix();

								for (std::size_t i = 0; i < 6; ++i) {
									shadowsCamera.setOrientation(kCubeMapOrientations[i]);
									shadowViewMatrix = shadowsCamera.getViewMatrix();

									shadowVPMatrices.push_back(shadowProjectionMatrix * shadowViewMatrix);
									light->getRenderable().setShadowViewMatrix(i, shadowViewMatrix);
									light->getRenderable().setShadowProjectionMatrix(i, shadowProjectionMatrix);
								}

								itUniforms->second.shadowVPMatrices.edit([=](auto& uniform) { uniform.setValue(shadowVPMatrices.data(), shadowVPMatrices.size()); });
								shadowVPMatrices.clear();
							}
						} break;
						case LightSource::Type::Spot: {
							float innerConeAngle, outerConeAngle;
							light->getSource()->getSpotLightRange(innerConeAngle, outerConeAngle);

							float radius = std::tan(outerConeAngle) * light->getSource()->getRange();
							float cosInner = std::cos(innerConeAngle);
							float cosOuter = std::cos(outerConeAngle);
							scale = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f * radius, light->getSource()->getRange(), 2.0f * radius));

							float lightAngleScale = 1.0f / std::max(0.001f, cosInner - cosOuter);
							float lightAngleOffset = -cosOuter * lightAngleScale;
							itUniforms->second.lightAngleScale.edit([=](auto& uniform) { uniform.setValue(lightAngleScale); });
							itUniforms->second.lightAngleOffset.edit([=](auto& uniform) { uniform.setValue(lightAngleOffset); });

							if (light->getRenderable().castsShadows()) {
								light->getSource()->getShadows(resolution, zNear, zFar, size, numCascades);

								shadowsCamera.setPosition(transforms->position);
								shadowsCamera.setOrientation(transforms->orientation);
								shadowsCamera.setPerspectiveProjection(2.0f * outerConeAngle, 1.0f, zNear, zFar);

								shadowViewMatrix = shadowsCamera.getViewMatrix();
								shadowProjectionMatrix = shadowsCamera.getProjectionMatrix();

								shadowVPMatrices.push_back(shadowProjectionMatrix * shadowViewMatrix);
								light->getRenderable().setShadowViewMatrix(0, shadowViewMatrix);
								light->getRenderable().setShadowProjectionMatrix(0, shadowProjectionMatrix);

								itUniforms->second.shadowVPMatrices.edit([=](auto& uniform) { uniform.setValue(shadowVPMatrices.data(), shadowVPMatrices.size()); });
								itUniforms->second.cascadesZFar.edit([=](auto& uniform) { uniform.setValue(&zFar, 1); });
								shadowVPMatrices.clear();
							}
						} break;
						default: {
							scale = glm::scale(glm::mat4(1.0f), glm::vec3(light->getSource()->getRange()));

							if (light->getRenderable().castsShadows()) {
								light->getSource()->getShadows(resolution, zNear, zFar, size, numCascades);

								// Get the zFar values of each cascade frustum
								auto depths = calculateCascadesZFar(camZNear, camZFar, numCascades);
								float previousDepth = camZNear;

								for (std::size_t i = 0; i < light->getRenderable().getNumShadows(); ++i) {
									// Calculate the cascade frustum corners in world space
									shadowsCamera.setPerspectiveProjection(camFOVY, camAspectRatio, previousDepth, depths[i]);
									glm::mat4 camInvViewProjMatrix = glm::inverse(shadowsCamera.getProjectionMatrix() * camViewMatrix);
									previousDepth = depths[i];

									glm::vec4 frustumCorners[] = {
										{-1, 1, 1, 1}, { 1, 1, 1, 1}, {-1,-1, 1, 1}, { 1,-1, 1, 1},
										{-1, 1,-1, 1}, { 1, 1,-1, 1}, {-1,-1,-1, 1}, { 1,-1,-1, 1}
									};
									glm::vec3 frustumCentroid(0.0f);

									for (glm::vec4& corner : frustumCorners) {
										corner = camInvViewProjMatrix * corner;
										corner /= corner.w;
										frustumCentroid += glm::vec3(corner);
									}

									frustumCentroid *= 1.0f / 8.0f;
									float frustumRadius = 0.5f * glm::length(frustumCorners[0] - frustumCorners[7]);

									// Calculate a view matrix used for moving the frustum center in texel sized increments
									// See https://alextardif.com/shadowmapping.html
									float texelsPerUnit = resolution / (2.0f * frustumRadius);
									glm::mat4 texelScaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(texelsPerUnit));

									shadowsCamera.setPosition(glm::vec3(0.0f));
									shadowsCamera.setOrientation(transforms->orientation);
									shadowViewMatrix = texelScaleMatrix * shadowsCamera.getViewMatrix();
									glm::mat4 invShadowViewMatrix = glm::inverse(shadowViewMatrix);

									// Move the frustum center using the previous matrix
									frustumCentroid = shadowViewMatrix * glm::vec4(frustumCentroid, 1.0f);
									frustumCentroid.x = floor(frustumCentroid.x);
									frustumCentroid.y = floor(frustumCentroid.y);
									frustumCentroid = invShadowViewMatrix * glm::vec4(frustumCentroid, 1.0f);

									// Calculate the view matrix using the new frustum center moved back in the light
									// direction by 2 * radius
									glm::vec3 shadowViewPosition = frustumCentroid - 2.0f * frustumRadius * (transforms->orientation * glm::vec3(0.0f, 0.0f, 1.0f));
									shadowsCamera.setPosition(shadowViewPosition);
									shadowsCamera.setOrientation(transforms->orientation);
									shadowViewMatrix = shadowsCamera.getViewMatrix();

									// Calculate the ortho projection matrix with the frustum radius. Multiply by the
									// zNear/zFar just for adding room for tall objects
									shadowsCamera.setOrthographicProjection(-frustumRadius, frustumRadius, -frustumRadius, frustumRadius, zNear * frustumRadius, zFar * frustumRadius);
									shadowProjectionMatrix = shadowsCamera.getProjectionMatrix();

									shadowVPMatrices.push_back(shadowProjectionMatrix * shadowViewMatrix);
									light->getRenderable().setShadowViewMatrix(i, shadowViewMatrix);
									light->getRenderable().setShadowProjectionMatrix(i, shadowProjectionMatrix);
								}

								itUniforms->second.shadowVPMatrices.edit([=](auto& uniform) { uniform.setValue(shadowVPMatrices.data(), shadowVPMatrices.size()); });
								itUniforms->second.cascadesZFar.edit([=](auto& uniform) { uniform.setValue(depths.data(), depths.size()); });
								shadowVPMatrices.clear();
							}
						} break;
					}

					glm::mat4 modelMatrix = translation * rotation * scale;
					light->getRenderable().getRenderableMesh().setModelMatrix(modelMatrix);
					for (std::size_t i = 0; i < 2; ++i) {
						itUniforms->second.modelMatrices[i].edit([=](auto& uniform) { uniform.setValue(modelMatrix); });
					}
				}
				, true
			);
		});

		SOMBRA_DEBUG_LOG << "Update end";
	}

// Private functions
	void LightSystem::onNewLight(Entity entity, LightComponent* light, EntityDatabase::Query&)
	{
		light->setup(&mApplication.getEventManager(), entity);

		setRMesh(entity, light);
		SOMBRA_INFO_LOG << "Entity " << entity << " with LightComponent " << light << " added successfully";
	}


	void LightSystem::onRemoveLight(Entity entity, LightComponent* light, EntityDatabase::Query&)
	{
		clearRMesh(entity, light);
		SOMBRA_INFO_LOG << "Entity " << entity << " with LightComponent " << light << " removed successfully";
	}


	void LightSystem::onNewMesh(Entity entity, MeshComponent* mesh, EntityDatabase::Query&)
	{
		mesh->processRenderableIndices([&, mesh = mesh](std::size_t i) {
			mesh->processRenderableShaders(i, [&](const auto& shader) {
				mShadowRenderSubGraph->getShadowUniformsUpdater().addRenderableTechnique(mesh->get(i), shader->getTechnique());
			});
		});
		SOMBRA_INFO_LOG << "Entity " << entity << " with MeshComponent " << mesh << " added successfully";
	}


	void LightSystem::onRemoveMesh(Entity entity, MeshComponent* mesh, EntityDatabase::Query&)
	{
		mesh->processRenderableIndices([&, mesh = mesh](std::size_t i) {
			mesh->processRenderableShaders(i, [&](const auto& shader) {
				mShadowRenderSubGraph->getShadowUniformsUpdater().removeRenderableTechnique(mesh->get(i), shader->getTechnique());
			});
		});
		SOMBRA_INFO_LOG << "Entity " << entity << " with MeshComponent " << mesh << " removed successfully";
	}


	void LightSystem::onNewTerrain(Entity entity, TerrainComponent* terrain, EntityDatabase::Query&)
	{
		terrain->processRenderableShaders([&](const auto& shader) {
			mShadowRenderSubGraph->getShadowUniformsUpdater().addRenderableTechnique(terrain->get(), shader->getTechnique());
		});
		SOMBRA_INFO_LOG << "Entity " << entity << " with TerrainComponent " << terrain << " added successfully";
	}


	void LightSystem::onRemoveTerrain(Entity entity, TerrainComponent* terrain, EntityDatabase::Query&)
	{
		terrain->processRenderableShaders([&](const auto& shader) {
			mShadowRenderSubGraph->getShadowUniformsUpdater().removeRenderableTechnique(terrain->get(), shader->getTechnique());
		});
		SOMBRA_INFO_LOG << "Entity " << entity << " with TerrainComponent " << terrain << " removed successfully";
	}


	void LightSystem::onCameraEvent(const ContainerEvent<Topic::Camera, Entity>& event)
	{
		SOMBRA_INFO_LOG << event;

		mEntityDatabase.executeQuery([&](EntityDatabase::Query& query) {
			auto [camera] = query.getComponents<CameraComponent>(event.getValue(), true);
			if (camera) {
				std::scoped_lock lock(mMutex);
				mCameraEntity = event.getValue();
				SOMBRA_INFO_LOG << "Entity " << mCameraEntity << " setted as camera";
			}
			else {
				SOMBRA_WARN_LOG << "Couldn't set Entity " << event.getValue() << " as Camera Entity";
			}
		});
	}


	void LightSystem::onRMeshEvent(const RMeshEvent& event)
	{
		SOMBRA_INFO_LOG << event;

		mEntityDatabase.executeQuery([&](EntityDatabase::Query& query) {
			auto [mesh] = query.getComponents<MeshComponent>(event.getEntity(), true);
			if (mesh) {
				switch (event.getOperation()) {
					case RMeshEvent::Operation::Add:
						mesh->processRenderableShaders(event.getRIndex(), [&](const auto& shader) {
							mShadowRenderSubGraph->getShadowUniformsUpdater().addRenderableTechnique(mesh->get(event.getRIndex()), shader->getTechnique());
						});
						break;
					case RMeshEvent::Operation::Remove:
						mesh->processRenderableShaders(event.getRIndex(), [&](const auto& shader) {
							mShadowRenderSubGraph->getShadowUniformsUpdater().removeRenderableTechnique(mesh->get(event.getRIndex()), shader->getTechnique());
						});
						break;
				}
			}
		});
	}


	void LightSystem::onLightSourceEvent(const LightSourceEvent& event)
	{
		SOMBRA_INFO_LOG << event;

		if (event.getEntity() != kNullEntity) {
			SOMBRA_TRACE_LOG << "Updating Entity " << event.getEntity();

			mEntityDatabase.executeQuery([&](EntityDatabase::Query& query) {
				auto [light] = query.getComponents<LightComponent>(event.getEntity());
				if (light) {
					clearRMesh(event.getEntity(), light);
					setRMesh(event.getEntity(), light);
				}
			});
		}
		else {
			SOMBRA_TRACE_LOG << "Updating LightComponents with source " << event.getLightSource();

			mEntityDatabase.executeQuery([&](EntityDatabase::Query& query) {
				query.iterateEntityComponents<LightComponent>(
					[&](Entity entity, LightComponent* light) {
						if (light->getSource().get() == event.getLightSource()) {
							clearRMesh(entity, light);
							setRMesh(entity, light);
						}
					},
					true
				);
			});
		}
	}


	void LightSystem::onRenderableShaderEvent(const RenderableShaderEvent& event)
	{
		SOMBRA_INFO_LOG << event;

		if (event.getRComponentType() == RenderableShaderEvent::RComponentType::Mesh) {
			mEntityDatabase.executeQuery([&](EntityDatabase::Query& query) {
				auto [mesh] = query.getComponents<MeshComponent>(event.getEntity(), true);
				if (mesh) {
					switch (event.getOperation()) {
						case RenderableShaderEvent::Operation::Add:
							mShadowRenderSubGraph->getShadowUniformsUpdater().addRenderableTechnique(mesh->get(event.getRIndex()), event.getShader()->getTechnique());
							break;
						case RenderableShaderEvent::Operation::Remove:
							mShadowRenderSubGraph->getShadowUniformsUpdater().removeRenderableTechnique(mesh->get(event.getRIndex()), event.getShader()->getTechnique());
							break;
					}
				}
			});
		}
		else {
			mEntityDatabase.executeQuery([&](EntityDatabase::Query& query) {
				graphics::Renderable* renderable = nullptr;
				if (event.getRComponentType() == RenderableShaderEvent::RComponentType::Terrain) {
					auto [terrain] = query.getComponents<TerrainComponent>(event.getEntity(), true);
					renderable = &terrain->get();
				}

				if (renderable) {
					switch (event.getOperation()) {
						case RenderableShaderEvent::Operation::Add:
							mShadowRenderSubGraph->getShadowUniformsUpdater().addRenderableTechnique(*renderable, event.getShader()->getTechnique());
							break;
						case RenderableShaderEvent::Operation::Remove:
							mShadowRenderSubGraph->getShadowUniformsUpdater().removeRenderableTechnique(*renderable, event.getShader()->getTechnique());
							break;
					}
				}
			});
		}
	}


	void LightSystem::onShaderEvent(const ShaderEvent& event)
	{
		SOMBRA_INFO_LOG << event;

		switch (event.getOperation()) {
			case ShaderEvent::Operation::Add:
				mShadowRenderSubGraph->getShadowUniformsUpdater().onAddTechniquePass(event.getShader()->getTechnique(), event.getStep()->getPass());
				break;
			case ShaderEvent::Operation::Remove:
				mShadowRenderSubGraph->getShadowUniformsUpdater().onRemoveTechniquePass(event.getShader()->getTechnique(), event.getStep()->getPass());
				break;
		}
	}


	void LightSystem::clearRMesh(Entity entity, LightComponent* light)
	{
		std::scoped_lock lock(mMutex);

		auto itEntityUniforms = mEntityUniforms.find(entity);
		if (itEntityUniforms == mEntityUniforms.end()) { return; }

		mApplication.getExternalTools().graphicsEngine->removeRenderable(&light->getRenderable());

		light->getRenderable()
			.disableShadows()
			.removeTechnique(mLightVolumeData->techniqueLight);
		light->removeRenderableShader(mLightVolumeData->shaderDeferredLighting[itEntityUniforms->second.iDL]);
		light->getRenderable().getRenderableMesh()
			.setMesh(nullptr)
			.clearBindables(mLightVolumeData->stepDeferredStencil->getPass().get())
			.clearBindables(mLightVolumeData->stepDeferredLighting[itEntityUniforms->second.iDL]->getPass().get());

		mEntityUniforms.erase(itEntityUniforms);
	}


	void LightSystem::setRMesh(Entity entity, LightComponent* light)
	{
		if (!light->getSource()) { return; }

		auto& context = mApplication.getExternalTools().graphicsEngine->getContext();

		std::scoped_lock lock(mMutex);
		auto itEntityUniforms = mEntityUniforms.emplace(entity, EntityUniforms()).first;

		auto mesh =	(light->getSource()->getType() == LightSource::Type::Point)? mLightVolumeData->pointLight :
					(light->getSource()->getType() == LightSource::Type::Spot)? mLightVolumeData->spotLight :
					mLightVolumeData->directionalLight;
		std::size_t iDL =	!light->getSource()->castsShadows()? LightVolumeData::kDL :
							(light->getSource()->getType() == LightSource::Type::Point)? LightVolumeData::kDLPLShadows :
							LightVolumeData::kDLCSM;

		light->getRenderable().getRenderableMesh().setMesh(mesh);
		light->addRenderableShader(mLightVolumeData->shaderDeferredLighting[iDL]);

		itEntityUniforms->second.iDL = iDL;
		itEntityUniforms->second.modelMatrices[0] = context.create<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix")
			.qedit([p = mLightVolumeData->programDeferredStencil](auto& q, auto& uniform) { uniform.load(*q.getTBindable(p)); });
		itEntityUniforms->second.modelMatrices[1] = context.create<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix")
			.qedit([p = mLightVolumeData->programDeferredLighting[iDL]](auto& q, auto& uniform) { uniform.load(*q.getTBindable(p)); });
		itEntityUniforms->second.type = context.create<graphics::UniformVariableValue<unsigned int>>("uBaseLight.type")
			.qedit([p = mLightVolumeData->programDeferredLighting[iDL]](auto& q, auto& uniform) { uniform.load(*q.getTBindable(p)); });
		itEntityUniforms->second.color = context.create<graphics::UniformVariableValue<glm::vec3>>("uBaseLight.color")
			.qedit([p = mLightVolumeData->programDeferredLighting[iDL]](auto& q, auto& uniform) { uniform.load(*q.getTBindable(p)); });
		itEntityUniforms->second.intensity = context.create<graphics::UniformVariableValue<float>>("uBaseLight.intensity")
			.qedit([p = mLightVolumeData->programDeferredLighting[iDL]](auto& q, auto& uniform) { uniform.load(*q.getTBindable(p)); });
		itEntityUniforms->second.range = context.create<graphics::UniformVariableValue<float>>("uBaseLight.range")
			.qedit([p = mLightVolumeData->programDeferredLighting[iDL]](auto& q, auto& uniform) { uniform.load(*q.getTBindable(p)); });
		itEntityUniforms->second.lightAngleScale = context.create<graphics::UniformVariableValue<float>>("uBaseLight.lightAngleScale")
			.qedit([p = mLightVolumeData->programDeferredLighting[iDL]](auto& q, auto& uniform) { uniform.load(*q.getTBindable(p)); });
		itEntityUniforms->second.lightAngleOffset = context.create<graphics::UniformVariableValue<float>>("uBaseLight.lightAngleOffset")
			.qedit([p = mLightVolumeData->programDeferredLighting[iDL]](auto& q, auto& uniform) { uniform.load(*q.getTBindable(p)); });

		light->getRenderable().getRenderableMesh()
			.addPassBindable(mLightVolumeData->stepDeferredStencil->getPass().get(), itEntityUniforms->second.modelMatrices[0])
			.addPassBindable(mLightVolumeData->stepDeferredLighting[iDL]->getPass().get(), itEntityUniforms->second.modelMatrices[1])
			.addPassBindable(mLightVolumeData->stepDeferredLighting[iDL]->getPass().get(), itEntityUniforms->second.type)
			.addPassBindable(mLightVolumeData->stepDeferredLighting[iDL]->getPass().get(), itEntityUniforms->second.color)
			.addPassBindable(mLightVolumeData->stepDeferredLighting[iDL]->getPass().get(), itEntityUniforms->second.intensity)
			.addPassBindable(mLightVolumeData->stepDeferredLighting[iDL]->getPass().get(), itEntityUniforms->second.range)
			.addPassBindable(mLightVolumeData->stepDeferredLighting[iDL]->getPass().get(), itEntityUniforms->second.lightAngleScale)
			.addPassBindable(mLightVolumeData->stepDeferredLighting[iDL]->getPass().get(), itEntityUniforms->second.lightAngleOffset);

		if (iDL != LightVolumeData::kDL) {
			itEntityUniforms->second.shadowVPMatrices = context.create<graphics::UniformVariableValueVector<glm::mat4>>("uShadowVPMatrices")
				.qedit([p = mLightVolumeData->programDeferredLighting[iDL]](auto& q, auto& uniform) { uniform.load(*q.getTBindable(p)); });
			light->getRenderable().getRenderableMesh()
				.addPassBindable(mLightVolumeData->stepDeferredLighting[iDL]->getPass().get(), itEntityUniforms->second.shadowVPMatrices);

			if (iDL == LightVolumeData::kDLCSM) {
				itEntityUniforms->second.numCascades = context.create<graphics::UniformVariableValue<unsigned int>>("uNumCascades")
					.qedit([p = mLightVolumeData->programDeferredLighting[iDL]](auto& q, auto& uniform) { uniform.load(*q.getTBindable(p)); });
				itEntityUniforms->second.cascadesZFar = context.create<graphics::UniformVariableValueVector<float>>("uCascadesZFar")
					.qedit([p = mLightVolumeData->programDeferredLighting[iDL]](auto& q, auto& uniform) { uniform.load(*q.getTBindable(p)); });

				light->getRenderable().getRenderableMesh()
					.addPassBindable(mLightVolumeData->stepDeferredLighting[iDL]->getPass().get(), itEntityUniforms->second.numCascades)
					.addPassBindable(mLightVolumeData->stepDeferredLighting[iDL]->getPass().get(), itEntityUniforms->second.cascadesZFar);
			}
		}

		light->getRenderable().addTechnique(mLightVolumeData->techniqueLight);
		if (light->getSource()->castsShadows()) {
			float size, zNear, zFar;
			std::size_t resolution, numCascades;
			light->getSource()->getShadows(resolution, zNear, zFar, size, numCascades);

			if (iDL == LightVolumeData::kDLPLShadows) {
				light->getRenderable().setShadows(context, resolution, true);
			}
			else {
				light->getRenderable().setShadows(context, resolution, false, numCascades);
				itEntityUniforms->second.numCascades.edit([=](auto& uniform) { uniform.setValue(static_cast<unsigned int>(numCascades)); });
			}
		}

		mApplication.getExternalTools().graphicsEngine->addRenderable(&light->getRenderable());
	}


	utils::FixedVector<float, RenderableLight::kMaxShadowMaps + 1>
		LightSystem::calculateCascadesZFar(float zNear, float zFar, std::size_t numCascades)
	{
		utils::FixedVector<float, RenderableLight::kMaxShadowMaps + 1> ret;

		for (std::size_t i = 1; i < numCascades; ++i) {
			ret.push_back(glm::lerp(
				zNear + i * (zFar - zNear) / numCascades,
				zNear * std::pow(zFar / zNear, i / static_cast<float>(numCascades)),
				mShadowSplitLogFactor
			));
		}
		ret.push_back(zFar);

		return ret;
	}

}
