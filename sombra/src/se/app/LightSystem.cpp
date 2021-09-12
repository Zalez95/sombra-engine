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
		using MeshRef = Repository::ResourceRef<graphics::Mesh>;
		using ProgramRef = Repository::ResourceRef<graphics::Program>;
		using PassRef = Repository::ResourceRef<graphics::Pass>;
		using TechniqueRef = Repository::ResourceRef<graphics::Technique>;
		using RenderableShaderStepRef = Repository::ResourceRef<RenderableShaderStep>;
		using RenderableShaderRef = Repository::ResourceRef<RenderableShader>;

		static constexpr std::size_t kDL = 0;
		static constexpr std::size_t kDLCSM = 1;
		static constexpr std::size_t kDLPLShadows = 2;
		static constexpr std::size_t kNumDL = 3;

		MeshRef pointLight, spotLight, directionalLight;
		PassRef passLight;
		TechniqueRef techniqueLight;

		ProgramRef programDeferredStencil;
		RenderableShaderStepRef stepDeferredStencil;

		ProgramRef programDeferredLighting[kNumDL];
		RenderableShaderStepRef stepDeferredLighting[kNumDL];
		RenderableShaderRef shaderDeferredLighting[kNumDL];
		UniformVVSPtr<glm::vec3> cameraPosition[kNumDL];
	};


	LightSystem::LightSystem(Application& application, float shadowSplitLogFactor) :
		ISystem(application.getEntityDatabase()), mApplication(application), mShadowSplitLogFactor(shadowSplitLogFactor)
	{
		mApplication.getEventManager()
			.subscribe(this, Topic::Camera)
			.subscribe(this, Topic::RendererResolution)
			.subscribe(this, Topic::RMesh)
			.subscribe(this, Topic::LightSource)
			.subscribe(this, Topic::RShader)
			.subscribe(this, Topic::Shader);
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask()
			.set<LightComponent>()
			.set<MeshComponent>()
			.set<TerrainComponent>()
		);

		mLightVolumeData = std::make_unique<LightVolumeData>();

		// Meshes
		mLightVolumeData->pointLight = mApplication.getRepository().findByName<graphics::Mesh>("pointLight");
		if (!mLightVolumeData->pointLight) {
			RawMesh rawMesh = MeshLoader::createSphereMesh("pointLight", 16, 10, 0.5f);
			rawMesh.texCoords.clear();
			auto [min, max] = MeshLoader::calculateBounds(rawMesh);
			graphics::Mesh gMesh = MeshLoader::createGraphicsMesh(rawMesh);
			gMesh.setBounds(min, max);
			mLightVolumeData->pointLight = mApplication.getRepository().insert(std::make_shared<graphics::Mesh>(std::move(gMesh)), "pointLight");
		}

		mLightVolumeData->spotLight = mApplication.getRepository().findByName<graphics::Mesh>("spotLight");
		if (!mLightVolumeData->spotLight) {
			RawMesh rawMesh = MeshLoader::createConeMesh("spotLight", 16, 0.5f, 1.0f);
			rawMesh.texCoords.clear();
			auto [min, max] = MeshLoader::calculateBounds(rawMesh);
			graphics::Mesh gMesh = MeshLoader::createGraphicsMesh(rawMesh);
			gMesh.setBounds(min, max);
			mLightVolumeData->spotLight = mApplication.getRepository().insert(std::make_shared<graphics::Mesh>(std::move(gMesh)), "spotLight");
		}

		mLightVolumeData->directionalLight = mApplication.getRepository().findByName<graphics::Mesh>("directionalLight");
		if (!mLightVolumeData->directionalLight) {
			RawMesh rawMesh = MeshLoader::createBoxMesh("directionalLight", glm::vec3(1.0f));
			rawMesh.texCoords.clear();
			auto [min, max] = MeshLoader::calculateBounds(rawMesh);
			graphics::Mesh gMesh = MeshLoader::createGraphicsMesh(rawMesh);
			gMesh.setBounds(min, max);
			mLightVolumeData->directionalLight = mApplication.getRepository().insert(std::make_shared<graphics::Mesh>(std::move(gMesh)), "directionalLight");
		}

		// Programs
		mLightVolumeData->programDeferredStencil = mApplication.getRepository().findByName<graphics::Program>("programDeferredStencil");
		if (!mLightVolumeData->programDeferredStencil) {
			std::shared_ptr<graphics::Program> program;
			auto result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, nullptr, program);
			if (!result) {
				SOMBRA_ERROR_LOG << result.description();
				return;
			}
			mLightVolumeData->programDeferredStencil = mApplication.getRepository().insert(std::move(program), "programDeferredStencil");
		}

		mLightVolumeData->programDeferredLighting[LightVolumeData::kDL] = mApplication.getRepository().findByName<graphics::Program>("programDeferredLighting");
		if (!mLightVolumeData->programDeferredLighting[LightVolumeData::kDL]) {
			std::shared_ptr<graphics::Program> program;
			auto result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentDeferredLighting.glsl", program);
			if (!result) {
				SOMBRA_ERROR_LOG << result.description();
				return;
			}
			mLightVolumeData->programDeferredLighting[LightVolumeData::kDL] = mApplication.getRepository().insert(std::move(program), "fragmentDeferredLighting");
		}

		mLightVolumeData->programDeferredLighting[LightVolumeData::kDLCSM] = mApplication.getRepository().findByName<graphics::Program>("programDeferredLightingCSM");
		if (!mLightVolumeData->programDeferredLighting[LightVolumeData::kDLCSM]) {
			std::shared_ptr<graphics::Program> program;
			auto result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentDeferredLightingCSM.glsl", program);
			if (!result) {
				SOMBRA_ERROR_LOG << result.description();
				return;
			}
			mLightVolumeData->programDeferredLighting[LightVolumeData::kDLCSM] = mApplication.getRepository().insert(std::move(program), "fragmentDeferredLightingCSM");
		}

		mLightVolumeData->programDeferredLighting[LightVolumeData::kDLPLShadows] = mApplication.getRepository().findByName<graphics::Program>("programDeferredLightingPLShadows");
		if (!mLightVolumeData->programDeferredLighting[LightVolumeData::kDLPLShadows]) {
			std::shared_ptr<graphics::Program> program;
			auto result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentDeferredLightingPLShadows.glsl", program);
			if (!result) {
				SOMBRA_ERROR_LOG << result.description();
				return;
			}
			mLightVolumeData->programDeferredLighting[LightVolumeData::kDLPLShadows] = mApplication.getRepository().insert(std::move(program), "fragmentDeferredLightingPLShadows");
		}

		// Passes
		auto& renderGraph = mApplication.getExternalTools().graphicsEngine->getRenderGraph();
		auto deferredLightSubGraph = dynamic_cast<DeferredLightSubGraph*>( renderGraph.getNode("deferredLightSubGraph") );
		mShadowRenderSubGraph = deferredLightSubGraph->getShadowRenderSubGraph();

		mLightVolumeData->passLight = mApplication.getRepository().findByName<graphics::Pass>("passLight");
		if (!mLightVolumeData->passLight) {
			mLightVolumeData->passLight = mApplication.getRepository().insert(std::make_shared<graphics::Pass>(*deferredLightSubGraph), "passLight");
		}

		// Techniques
		mLightVolumeData->techniqueLight = mApplication.getRepository().findByName<graphics::Technique>("techniqueLight");
		if (!mLightVolumeData->techniqueLight) {
			mLightVolumeData->techniqueLight = mApplication.getRepository().insert(std::make_shared<graphics::Technique>(), "techniqueLight");
			mLightVolumeData->techniqueLight->addPass(mLightVolumeData->passLight.get());
		}

		// RenderableShaderSteps
		mLightVolumeData->stepDeferredStencil = mApplication.getRepository().findByName<RenderableShaderStep>("stepDeferredStencil");
		if (!mLightVolumeData->stepDeferredStencil) {
			mLightVolumeData->stepDeferredStencil = mApplication.getRepository().insert(std::make_shared<RenderableShaderStep>(*deferredLightSubGraph->getStencilRenderer()), "stepDeferredStencil");
			mLightVolumeData->stepDeferredStencil->addBindable(mLightVolumeData->programDeferredStencil.get());
		}

		mLightVolumeData->stepDeferredLighting[LightVolumeData::kDL] = mApplication.getRepository().findByName<RenderableShaderStep>("stepDeferredLighting");
		if (!mLightVolumeData->stepDeferredLighting[LightVolumeData::kDL]) {
			auto program = mLightVolumeData->programDeferredLighting[LightVolumeData::kDL].get();
			mLightVolumeData->cameraPosition[LightVolumeData::kDL] = std::make_shared<graphics::UniformVariableValue<glm::vec3>>("uViewPosition", program);

			mLightVolumeData->stepDeferredLighting[LightVolumeData::kDL] = mApplication.getRepository().insert(std::make_shared<RenderableShaderStep>(*deferredLightSubGraph->getColorRenderer()), "stepDeferredLighting");
			mLightVolumeData->stepDeferredLighting[LightVolumeData::kDL]->addBindable(program)
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uPosition", program, DeferredLightSubGraph::TexUnits::kPosition))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uNormal", program, DeferredLightSubGraph::TexUnits::kNormal))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uAlbedo", program, DeferredLightSubGraph::TexUnits::kAlbedo))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uMaterial", program, DeferredLightSubGraph::TexUnits::kMaterial))
				.addBindable(mLightVolumeData->cameraPosition[LightVolumeData::kDL]);
		}

		mLightVolumeData->stepDeferredLighting[LightVolumeData::kDLCSM] = mApplication.getRepository().findByName<RenderableShaderStep>("stepDeferredLightingCSM");
		if (!mLightVolumeData->stepDeferredLighting[LightVolumeData::kDLCSM]) {
			auto program = mLightVolumeData->programDeferredLighting[LightVolumeData::kDLCSM].get();
			mLightVolumeData->cameraPosition[LightVolumeData::kDLCSM] = std::make_shared<graphics::UniformVariableValue<glm::vec3>>("uViewPosition", program);

			mLightVolumeData->stepDeferredLighting[LightVolumeData::kDLCSM] = mApplication.getRepository().insert(std::make_shared<RenderableShaderStep>(*deferredLightSubGraph->getColorRenderer()), "stepDeferredLightingCSM");
			mLightVolumeData->stepDeferredLighting[LightVolumeData::kDLCSM]->addBindable(program)
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uPosition", program, DeferredLightSubGraph::TexUnits::kPosition))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uNormal", program, DeferredLightSubGraph::TexUnits::kNormal))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uAlbedo", program, DeferredLightSubGraph::TexUnits::kAlbedo))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uMaterial", program, DeferredLightSubGraph::TexUnits::kMaterial))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uShadow", program, DeferredLightSubGraph::TexUnits::kShadow))
				.addBindable(mLightVolumeData->cameraPosition[LightVolumeData::kDLCSM]);
		}

		mLightVolumeData->stepDeferredLighting[LightVolumeData::kDLPLShadows] = mApplication.getRepository().findByName<RenderableShaderStep>("stepDeferredLightingPLShadows");
		if (!mLightVolumeData->stepDeferredLighting[LightVolumeData::kDLPLShadows]) {
			auto program = mLightVolumeData->programDeferredLighting[LightVolumeData::kDLPLShadows].get();
			mLightVolumeData->cameraPosition[LightVolumeData::kDLPLShadows] = std::make_shared<graphics::UniformVariableValue<glm::vec3>>("uViewPosition", program);

			mLightVolumeData->stepDeferredLighting[LightVolumeData::kDLPLShadows] = mApplication.getRepository().insert(std::make_shared<RenderableShaderStep>(*deferredLightSubGraph->getColorRenderer()), "stepDeferredLightingPLShadows");
			mLightVolumeData->stepDeferredLighting[LightVolumeData::kDLPLShadows]->addBindable(program)
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uPosition", program, DeferredLightSubGraph::TexUnits::kPosition))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uNormal", program, DeferredLightSubGraph::TexUnits::kNormal))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uAlbedo", program, DeferredLightSubGraph::TexUnits::kAlbedo))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uMaterial", program, DeferredLightSubGraph::TexUnits::kMaterial))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uShadow", program, DeferredLightSubGraph::TexUnits::kShadow))
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


	void LightSystem::onNewComponent(Entity entity, const EntityDatabase::ComponentMask& mask)
	{
		tryCallC(&LightSystem::onNewLight, entity, mask);
		tryCallC(&LightSystem::onNewMesh, entity, mask);
		tryCallC(&LightSystem::onNewTerrain, entity, mask);
	}


	void LightSystem::onRemoveComponent(Entity entity, const EntityDatabase::ComponentMask& mask)
	{
		tryCallC(&LightSystem::onRemoveLight, entity, mask);
		tryCallC(&LightSystem::onRemoveMesh, entity, mask);
		tryCallC(&LightSystem::onRemoveTerrain, entity, mask);
	}


	void LightSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Lights";

		CameraComponent shadowsCamera;
		glm::mat4 shadowViewMatrix, shadowProjectionMatrix, camViewMatrix(1.0f);
		utils::FixedVector<glm::mat4, RenderableLight::kMaxShadowMaps> shadowVPMatrices;
		float size, zNear, zFar, camFOVY(0.0f), camAspectRatio(1.0f), camZNear(-1.0f), camZFar(1.0f);
		std::size_t resolution, numCascades;

		if (mCameraEntity != kNullEntity) {
			auto [transforms, camera] = mEntityDatabase.getComponents<TransformsComponent, CameraComponent>(mCameraEntity, true);
			if (transforms && camera) {
				for (std::size_t i = 0; i < LightVolumeData::kNumDL; ++i) {
					mLightVolumeData->cameraPosition[i]->setValue(transforms->position);
				}

				if (!camera->hasOrthographicProjection()) {
					camera->getPerspectiveParams(camFOVY, camAspectRatio, camZNear, camZFar);
					camViewMatrix = camera->getViewMatrix();
				}
			}
		}

		for (auto& [entity, entityUniforms] : mEntityUniforms) {
			auto [light, transforms] = mApplication.getEntityDatabase().getComponents<LightComponent, TransformsComponent>(entity);

			glm::mat4 translation(1.0f), rotation(1.0f), scale(1.0f);
			if (transforms) {
				translation = glm::translate(glm::mat4(1.0f), transforms->position);
				rotation = glm::mat4_cast(transforms->orientation);
			}

			entityUniforms.type->setValue(static_cast<int>(light->getSource()->getType()));
			entityUniforms.color->setValue(light->getSource()->getColor());
			entityUniforms.intensity->setValue(light->getSource()->getIntensity());
			entityUniforms.range->setValue(light->getSource()->getRange());

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

						entityUniforms.shadowVPMatrices->setValue(shadowVPMatrices.data(), shadowVPMatrices.size());
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

					entityUniforms.lightAngleScale->setValue(1.0f / std::max(0.001f, cosInner - cosOuter));
					entityUniforms.lightAngleOffset->setValue(-cosOuter * entityUniforms.lightAngleScale->getValue());

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

						entityUniforms.shadowVPMatrices->setValue(shadowVPMatrices.data(), shadowVPMatrices.size());
						entityUniforms.cascadesZFar->setValue(&zFar, 1);
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

						entityUniforms.shadowVPMatrices->setValue(shadowVPMatrices.data(), shadowVPMatrices.size());
						entityUniforms.cascadesZFar->setValue(depths.data(), depths.size());
						shadowVPMatrices.clear();
					}
				} break;
			}

			glm::mat4 modelMatrix = translation * rotation * scale;
			light->getRenderable().getRenderableMesh().setModelMatrix(modelMatrix);
			for (std::size_t i = 0; i < 2; ++i) {
				entityUniforms.modelMatrices[i]->setValue(modelMatrix);
			}
		}

		SOMBRA_DEBUG_LOG << "Update end";
	}

// Private functions
	void LightSystem::onNewLight(Entity entity, LightComponent* light)
	{
		light->setup(&mApplication.getEventManager(), entity);

		setRMesh(entity, light);
		SOMBRA_INFO_LOG << "Entity " << entity << " with LightComponent " << light << " added successfully";
	}


	void LightSystem::onRemoveLight(Entity entity, LightComponent* light)
	{
		clearRMesh(entity, light);
		SOMBRA_INFO_LOG << "Entity " << entity << " with LightComponent " << light << " removed successfully";
	}


	void LightSystem::onNewMesh(Entity entity, MeshComponent* mesh)
	{
		mesh->processRenderableIndices([&, mesh = mesh](std::size_t i) {
			mesh->processRenderableShaders(i, [&](const auto& shader) {
				mShadowRenderSubGraph->getShadowUniformsUpdater().addRenderableTechnique(mesh->get(i), shader->getTechnique());
			});
		});
		SOMBRA_INFO_LOG << "Entity " << entity << " with MeshComponent " << mesh << " added successfully";
	}


	void LightSystem::onRemoveMesh(Entity entity, MeshComponent* mesh)
	{
		mesh->processRenderableIndices([&, mesh = mesh](std::size_t i) {
			mShadowRenderSubGraph->getShadowUniformsUpdater().removeRenderable(mesh->get(i));
		});
		SOMBRA_INFO_LOG << "Entity " << entity << " with MeshComponent " << mesh << " removed successfully";
	}


	void LightSystem::onNewTerrain(Entity entity, TerrainComponent* terrain)
	{
		terrain->processRenderableShaders([&](const auto& shader) {
			mShadowRenderSubGraph->getShadowUniformsUpdater().addRenderableTechnique(terrain->get(), shader->getTechnique());
		});
		SOMBRA_INFO_LOG << "Entity " << entity << " with TerrainComponent " << terrain << " added successfully";
	}


	void LightSystem::onRemoveTerrain(Entity entity, TerrainComponent* terrain)
	{
		mShadowRenderSubGraph->getShadowUniformsUpdater().removeRenderable(terrain->get());
		SOMBRA_INFO_LOG << "Entity " << entity << " with TerrainComponent " << terrain << " removed successfully";
	}


	void LightSystem::onCameraEvent(const ContainerEvent<Topic::Camera, Entity>& event)
	{
		SOMBRA_INFO_LOG << event;

		auto [camera] = mEntityDatabase.getComponents<CameraComponent>(event.getValue(), true);
		if (camera) {
			mCameraEntity = event.getValue();
			SOMBRA_INFO_LOG << "Entity " << mCameraEntity << " setted as camera";
		}
		else {
			SOMBRA_WARN_LOG << "Couldn't set Entity " << event.getValue() << " as Camera Entity";
		}
	}


	void LightSystem::onRMeshEvent(const RMeshEvent& event)
	{
		SOMBRA_INFO_LOG << event;

		auto [mesh] = mEntityDatabase.getComponents<MeshComponent>(event.getEntity(), true);
		if (mesh) {
			switch (event.getOperation()) {
				case RMeshEvent::Operation::Add:
					mesh->processRenderableShaders(event.getRIndex(), [&](const auto& shader) {
						mShadowRenderSubGraph->getShadowUniformsUpdater().addRenderableTechnique(mesh->get(event.getRIndex()), shader->getTechnique());
					});
					break;
				case RMeshEvent::Operation::Remove:
					mShadowRenderSubGraph->getShadowUniformsUpdater().removeRenderable(mesh->get(event.getRIndex()));
					break;
			}
		}
	}


	void LightSystem::onLightSourceEvent(const LightSourceEvent& event)
	{
		SOMBRA_INFO_LOG << event;

		if (event.getEntity() != kNullEntity) {
			SOMBRA_TRACE_LOG << "Updating Entity " << event.getEntity();

			auto [light] = mEntityDatabase.getComponents<LightComponent>(event.getEntity());
			if (light) {
				clearRMesh(event.getEntity(), light);
				setRMesh(event.getEntity(), light);
			}
		}
		else {
			SOMBRA_TRACE_LOG << "Updating LightComponents with source " << event.getLightSource();

			mEntityDatabase.iterateComponents<LightComponent>(
				[&](Entity entity, LightComponent* light) {
					if (light->getSource().get() == event.getLightSource()) {
						clearRMesh(entity, light);
						setRMesh(entity, light);
					}
				},
				true
			);
		}
	}


	void LightSystem::onRenderableShaderEvent(const RenderableShaderEvent& event)
	{
		SOMBRA_INFO_LOG << event;

		if (event.getRComponentType() == RenderableShaderEvent::RComponentType::Mesh) {
			auto [mesh] = mEntityDatabase.getComponents<MeshComponent>(event.getEntity(), true);
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
		}
		else {
			graphics::Renderable* renderable = nullptr;
			if (event.getRComponentType() == RenderableShaderEvent::RComponentType::Terrain) {
				auto [terrain] = mEntityDatabase.getComponents<TerrainComponent>(event.getEntity(), true);
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
		auto itEntityUniforms = mEntityUniforms.find(entity);
		if (itEntityUniforms == mEntityUniforms.end()) { return; }

		mApplication.getExternalTools().graphicsEngine->removeRenderable(&light->getRenderable());

		light->getRenderable()
			.disableShadows()
			.removeTechnique(mLightVolumeData->techniqueLight.get());
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

		auto itEntityUniforms = mEntityUniforms.emplace(entity, EntityUniforms()).first;

		auto mesh =	(light->getSource()->getType() == LightSource::Type::Point)? mLightVolumeData->pointLight.get() :
					(light->getSource()->getType() == LightSource::Type::Spot)? mLightVolumeData->spotLight.get() :
					mLightVolumeData->directionalLight.get();
		std::size_t iDL =	!light->getSource()->castsShadows()? LightVolumeData::kDL :
							(light->getSource()->getType() == LightSource::Type::Point)? LightVolumeData::kDLPLShadows :
							LightVolumeData::kDLCSM;

		light->getRenderable().getRenderableMesh().setMesh(mesh);
		light->addRenderableShader(mLightVolumeData->shaderDeferredLighting[iDL]);

		itEntityUniforms->second.iDL = iDL;
		itEntityUniforms->second.modelMatrices[0] = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", mLightVolumeData->programDeferredStencil.get());
		itEntityUniforms->second.modelMatrices[1] = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", mLightVolumeData->programDeferredLighting[iDL].get());
		itEntityUniforms->second.type = std::make_shared<graphics::UniformVariableValue<unsigned int>>("uBaseLight.type", mLightVolumeData->programDeferredLighting[iDL].get());
		itEntityUniforms->second.color = std::make_shared<graphics::UniformVariableValue<glm::vec3>>("uBaseLight.color", mLightVolumeData->programDeferredLighting[iDL].get());
		itEntityUniforms->second.intensity = std::make_shared<graphics::UniformVariableValue<float>>("uBaseLight.intensity", mLightVolumeData->programDeferredLighting[iDL].get());
		itEntityUniforms->second.range = std::make_shared<graphics::UniformVariableValue<float>>("uBaseLight.range", mLightVolumeData->programDeferredLighting[iDL].get());
		itEntityUniforms->second.lightAngleScale = std::make_shared<graphics::UniformVariableValue<float>>("uBaseLight.lightAngleScale", mLightVolumeData->programDeferredLighting[iDL].get());
		itEntityUniforms->second.lightAngleOffset = std::make_shared<graphics::UniformVariableValue<float>>("uBaseLight.lightAngleOffset", mLightVolumeData->programDeferredLighting[iDL].get());

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
			itEntityUniforms->second.shadowVPMatrices = std::make_shared<graphics::UniformVariableValueVector<glm::mat4>>("uShadowVPMatrices", mLightVolumeData->programDeferredLighting[iDL].get());
			light->getRenderable().getRenderableMesh()
				.addPassBindable(mLightVolumeData->stepDeferredLighting[iDL]->getPass().get(), itEntityUniforms->second.shadowVPMatrices);

			if (iDL == LightVolumeData::kDLCSM) {
				itEntityUniforms->second.numCascades = std::make_shared<graphics::UniformVariableValue<unsigned int>>("uNumCascades", mLightVolumeData->programDeferredLighting[iDL].get());
				itEntityUniforms->second.cascadesZFar = std::make_shared<graphics::UniformVariableValueVector<float>>("uCascadesZFar", mLightVolumeData->programDeferredLighting[iDL].get());

				light->getRenderable().getRenderableMesh()
					.addPassBindable(mLightVolumeData->stepDeferredLighting[iDL]->getPass().get(), itEntityUniforms->second.numCascades)
					.addPassBindable(mLightVolumeData->stepDeferredLighting[iDL]->getPass().get(), itEntityUniforms->second.cascadesZFar);
			}
		}

		light->getRenderable().addTechnique(mLightVolumeData->techniqueLight.get());
		if (light->getSource()->castsShadows()) {
			float size, zNear, zFar;
			std::size_t resolution, numCascades;
			light->getSource()->getShadows(resolution, zNear, zFar, size, numCascades);

			if (iDL == LightVolumeData::kDLPLShadows) {
				light->getRenderable().setShadows(resolution, true);
			}
			else {
				light->getRenderable().setShadows(resolution, false, numCascades);
				itEntityUniforms->second.numCascades->setValue(static_cast<unsigned int>(numCascades));
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
