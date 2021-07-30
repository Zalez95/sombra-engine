#include "se/utils/Log.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/graphics/Renderer.h"
#include "se/app/io/MeshLoader.h"
#include "se/app/io/ShaderLoader.h"
#include "se/app/LightSystem.h"
#include "se/app/Application.h"
#include "se/app/Repository.h"
#include "se/app/LightComponent.h"
#include "se/app/TransformsComponent.h"
#include "graphics/DeferredLightSubGraph.h"

namespace se::app {

	LightSystem::LightSystem(Application& application, std::size_t width, std::size_t height) :
		ISystem(application.getEntityDatabase()), mApplication(application), mCameraEntity(kNullEntity)
	{
		mApplication.getEventManager()
			.subscribe(this, Topic::Camera)
			.subscribe(this, Topic::RendererResolution);
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask().set<LightComponent>());

		mProgramsDeferred[0] = mApplication.getRepository().findByName<graphics::Program>("programDeferredStencil");
		if (!mProgramsDeferred[0]) {
			std::shared_ptr<graphics::Program> program;
			auto result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, nullptr, program);
			if (!result) {
				SOMBRA_ERROR_LOG << result.description();
				return;
			}
			mProgramsDeferred[0] = mApplication.getRepository().insert(std::move(program), "programDeferredStencil");
		}

		mProgramsDeferred[1] = mApplication.getRepository().findByName<graphics::Program>("fragmentDeferredLighting");
		if (!mProgramsDeferred[1]) {
			std::shared_ptr<graphics::Program> program;
			auto result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentDeferredLighting.glsl", program);
			if (!result) {
				SOMBRA_ERROR_LOG << result.description();
				return;
			}
			mProgramsDeferred[1] = mApplication.getRepository().insert(std::move(program), "fragmentDeferredLighting");
		}

		auto& renderGraph = mApplication.getExternalTools().graphicsEngine->getRenderGraph();

		mPassesDeferred[0] = mApplication.getRepository().findByName<graphics::Pass>("passDeferredStencil");
		if (!mPassesDeferred[0]) {
			auto lightStencilRenderer = dynamic_cast<graphics::Renderer*>(renderGraph.getNode("lightStencilRenderer"));
			mPassesDeferred[0] = mApplication.getRepository().insert(std::make_shared<graphics::Pass>(*lightStencilRenderer));
			mPassesDeferred[0]->addBindable(mProgramsDeferred[0].get());
		}

		mPassesDeferred[1] = mApplication.getRepository().findByName<graphics::Pass>("passDeferredColor");
		if (!mPassesDeferred[1]) {
			auto lightColorRenderer = dynamic_cast<graphics::Renderer*>(renderGraph.getNode("lightColorRenderer"));
			mCameraPosition = std::make_shared<graphics::UniformVariableValue<glm::vec3>>("uViewPosition", mProgramsDeferred[1].get());

			mPassesDeferred[1] = mApplication.getRepository().insert(std::make_shared<graphics::Pass>(*lightColorRenderer));
			mPassesDeferred[1]->addBindable(mProgramsDeferred[1].get())
				.addBindable(std::make_shared<graphics::UniformVariableValue<glm::vec2>>("uResolution", mProgramsDeferred[1].get(), glm::vec2(width, height)))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uPosition", mProgramsDeferred[1].get(), DeferredLightSubGraph::TexUnits::kPosition))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uNormal", mProgramsDeferred[1].get(), DeferredLightSubGraph::TexUnits::kNormal))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uAlbedo", mProgramsDeferred[1].get(), DeferredLightSubGraph::TexUnits::kAlbedo))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uMaterial", mProgramsDeferred[1].get(), DeferredLightSubGraph::TexUnits::kMaterial))
				.addBindable(mCameraPosition);
		}

		mTechniqueDeferred = mApplication.getRepository().findByName<graphics::Technique>("techniqueDeferredLighting");
		if (!mTechniqueDeferred) {
			mTechniqueDeferred = mApplication.getRepository().insert(std::make_shared<graphics::Technique>());
			mTechniqueDeferred->addPass(mPassesDeferred[0].get())
				.addPass(mPassesDeferred[1].get());
		}

		mPointLight = mApplication.getRepository().findByName<graphics::Mesh>("pointLight");
		if (!mPointLight) {
			RawMesh rawMesh = MeshLoader::createSphereMesh("pointLight", 16, 10, 0.5f);
			rawMesh.texCoords.clear();
			auto [min, max] = MeshLoader::calculateBounds(rawMesh);
			graphics::Mesh gMesh = MeshLoader::createGraphicsMesh(rawMesh);
			gMesh.setBounds(min, max);
			mPointLight = mApplication.getRepository().insert(std::make_shared<graphics::Mesh>(std::move(gMesh)), "pointLight");
		}

		mSpotLight = mApplication.getRepository().findByName<graphics::Mesh>("spotLight");
		if (!mSpotLight) {
			RawMesh rawMesh = MeshLoader::createConeMesh("spotLight", 16, 0.5f, 1.0f);
			rawMesh.texCoords.clear();
			auto [min, max] = MeshLoader::calculateBounds(rawMesh);
			graphics::Mesh gMesh = MeshLoader::createGraphicsMesh(rawMesh);
			gMesh.setBounds(min, max);
			mSpotLight = mApplication.getRepository().insert(std::make_shared<graphics::Mesh>(std::move(gMesh)), "spotLight");
		}

		mDirectionalLight = mApplication.getRepository().findByName<graphics::Mesh>("directionalLight");
		if (!mDirectionalLight) {
			RawMesh rawMesh = MeshLoader::createBoxMesh("directionalLight", glm::vec3(1.0f));
			rawMesh.texCoords.clear();
			auto [min, max] = MeshLoader::calculateBounds(rawMesh);
			graphics::Mesh gMesh = MeshLoader::createGraphicsMesh(rawMesh);
			gMesh.setBounds(min, max);
			mDirectionalLight = mApplication.getRepository().insert(std::make_shared<graphics::Mesh>(std::move(gMesh)), "directionalLight");
		}
	}


	LightSystem::~LightSystem()
	{
		mEntityDatabase.removeSystem(this);
		mApplication.getEventManager()
			.unsubscribe(this, Topic::RendererResolution)
			.unsubscribe(this, Topic::Camera);
	}


	bool LightSystem::notify(const IEvent& event)
	{
		return tryCall(&LightSystem::onCameraEvent, event)
			|| tryCall(&LightSystem::onRendererResolutionEvent, event);
	}


	void LightSystem::onNewComponent(Entity entity, const EntityDatabase::ComponentMask& mask)
	{
		tryCallC(&LightSystem::onNewLight, entity, mask);
	}


	void LightSystem::onRemoveComponent(Entity entity, const EntityDatabase::ComponentMask& mask)
	{
		tryCallC(&LightSystem::onRemoveLight, entity, mask);
	}


	void LightSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Renderers";

		for (auto& [entity, entityUniforms] : mEntityUniforms) {
			auto [light, transforms] = mApplication.getEntityDatabase().getComponents<LightComponent, TransformsComponent>(entity);

			glm::mat4 translation(1.0f), rotation(1.0f), scale(1.0f);
			if (transforms) {
				translation = glm::translate(glm::mat4(1.0f), transforms->position);
				rotation = glm::mat4_cast(transforms->orientation);
			}

			entityUniforms.type->setValue(static_cast<int>(light->getSource()->type));
			entityUniforms.color->setValue(light->getSource()->color);
			entityUniforms.intensity->setValue(light->getSource()->intensity);
			entityUniforms.range->setValue(light->getSource()->range);

			switch (light->getSource()->type) {
				case LightSource::Type::Point: {
					light->getLightRenderable().setMesh(mPointLight.get());
					scale = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f * light->getSource()->range));
				} break;
				case LightSource::Type::Spot: {
					light->getLightRenderable().setMesh(mSpotLight.get());

					float radius = std::tan(light->getSource()->outerConeAngle) * light->getSource()->range;
					float cosInner = std::cos(light->getSource()->innerConeAngle);
					float cosOuter = std::cos(light->getSource()->outerConeAngle);
					scale = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f * radius, light->getSource()->range, 2.0f * radius));

					entityUniforms.lightAngleScale->setValue(1.0f / std::max(0.001f, cosInner - cosOuter));
					entityUniforms.lightAngleOffset->setValue(-cosOuter * entityUniforms.lightAngleScale->getValue());
				} break;
				default: {
					light->getLightRenderable().setMesh(mDirectionalLight.get());
					scale = glm::scale(glm::mat4(1.0f), glm::vec3(light->getSource()->range));
				} break;
			}

			glm::mat4 modelMatrix = translation * rotation * scale;
			light->getLightRenderable().setModelMatrix(modelMatrix);
			for (std::size_t i = 0; i < 2; ++i) {
				entityUniforms.modelMatrices[i]->setValue(modelMatrix);
			}
		}

		if (mCameraEntity != kNullEntity) {
			auto [camTransforms] = mEntityDatabase.getComponents<TransformsComponent>(mCameraEntity, true);
			if (camTransforms) {
				mCameraPosition->setValue(camTransforms->position);
			}
		}

		SOMBRA_INFO_LOG << "Update end";
	}

// Private functions
	void LightSystem::onNewLight(Entity entity, LightComponent* light)
	{
		light->setup(&mApplication.getEventManager(), entity);
		light->getLightRenderable().setMesh(mPointLight.get())
			.addTechnique(mTechniqueDeferred.get());

		auto itEntityUniforms = mEntityUniforms.emplace(entity, EntityUniforms()).first;
		for (std::size_t i = 0; i < 2; ++i) {
			itEntityUniforms->second.modelMatrices[i] = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", mProgramsDeferred[i].get());
			light->getLightRenderable().addPassBindable(&(*mPassesDeferred[i]), itEntityUniforms->second.modelMatrices[i]);
		}
		itEntityUniforms->second.type = std::make_shared<graphics::UniformVariableValue<unsigned int>>("uBaseLight.type", mProgramsDeferred[1].get());
		itEntityUniforms->second.color = std::make_shared<graphics::UniformVariableValue<glm::vec3>>("uBaseLight.color", mProgramsDeferred[1].get());
		itEntityUniforms->second.intensity = std::make_shared<graphics::UniformVariableValue<float>>("uBaseLight.intensity", mProgramsDeferred[1].get());
		itEntityUniforms->second.range = std::make_shared<graphics::UniformVariableValue<float>>("uBaseLight.range", mProgramsDeferred[1].get());
		itEntityUniforms->second.lightAngleScale = std::make_shared<graphics::UniformVariableValue<float>>("uBaseLight.lightAngleScale", mProgramsDeferred[1].get());
		itEntityUniforms->second.lightAngleOffset = std::make_shared<graphics::UniformVariableValue<float>>("uBaseLight.lightAngleOffset", mProgramsDeferred[1].get());
		light->getLightRenderable().addPassBindable(&(*mPassesDeferred[1]), itEntityUniforms->second.type);
		light->getLightRenderable().addPassBindable(&(*mPassesDeferred[1]), itEntityUniforms->second.color);
		light->getLightRenderable().addPassBindable(&(*mPassesDeferred[1]), itEntityUniforms->second.intensity);
		light->getLightRenderable().addPassBindable(&(*mPassesDeferred[1]), itEntityUniforms->second.range);
		light->getLightRenderable().addPassBindable(&(*mPassesDeferred[1]), itEntityUniforms->second.lightAngleScale);
		light->getLightRenderable().addPassBindable(&(*mPassesDeferred[1]), itEntityUniforms->second.lightAngleOffset);

		mApplication.getExternalTools().graphicsEngine->addRenderable(&light->getLightRenderable());

		SOMBRA_INFO_LOG << "Entity " << entity << " with LightComponent " << light << " added successfully";
	}


	void LightSystem::onRemoveLight(Entity entity, LightComponent* light)
	{
		mApplication.getExternalTools().graphicsEngine->removeRenderable(&light->getLightRenderable());

		auto itEntityUniforms = mEntityUniforms.find(entity);
		if (itEntityUniforms != mEntityUniforms.end()) {
			for (std::size_t i = 0; i < 2; ++i) {
				light->getLightRenderable().clearBindables(&(*mPassesDeferred[i]));
			}
			mEntityUniforms.erase(itEntityUniforms);
		}

		light->getLightRenderable().setMesh(nullptr)
			.removeTechnique(mTechniqueDeferred.get());
		light->setup(nullptr, kNullEntity);

		SOMBRA_INFO_LOG << "Entity " << entity << " with LightComponent " << light << " removed successfully";
	}


	void LightSystem::onCameraEvent(const ContainerEvent<Topic::Camera, Entity>& event)
	{
		mCameraEntity = event.getValue();

		SOMBRA_INFO_LOG << "Entity " << mCameraEntity << " setted as camera";
	}


	void LightSystem::onRendererResolutionEvent(const RendererResolutionEvent& event)
	{
		if (mPassesDeferred[1]) {
			mPassesDeferred[1]->processBindables([&](const std::shared_ptr<graphics::Bindable>& bindable) {
				if (auto uniform = std::dynamic_pointer_cast<graphics::UniformVariableValue<glm::vec2>>(bindable)) {
					if (uniform->getName() == "uResolution") {
						float width = static_cast<float>(event.getWidth());
						float height = static_cast<float>(event.getHeight());
						uniform->setValue({ width, height });
					}
				}
			});
		}
	}

}
