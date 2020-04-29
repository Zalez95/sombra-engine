#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "se/app/GraphicsManager.h"
#include "se/app/events/ResizeEvent.h"
#include "se/utils/Log.h"
#include "se/graphics/2D/Step2D.h"
#include "se/graphics/3D/Step3D.h"
#include "se/graphics/core/Program.h"
#include "se/graphics/core/GraphicsOperations.h"

namespace se::app {

	struct ShaderPointLight
	{
		glm::vec3 color;
		float intensity;
		float inverseRange;
		glm::vec3 padding = glm::vec3(0.0f);
	};


	struct GraphicsManager::Impl
	{
		struct RenderableMeshData
		{
			RenderableMeshUPtr renderable;
			std::vector<std::shared_ptr<graphics::UniformVariableValue<glm::mat4>>> modelMatrix;
			SkinSPtr skin;
			std::vector<std::shared_ptr<graphics::UniformVariableValueVector<glm::mat4, kMaxJoints>>> jointMatrices;

			RenderableMeshData(RenderableMeshUPtr renderable) :
				renderable(std::move(renderable)), skin(nullptr) {};
		};

		struct RenderableTerrainData
		{
			RenderableTerrainUPtr renderable;
			std::vector<std::shared_ptr<graphics::UniformVariableValue<glm::mat4>>> modelMatrix;

			RenderableTerrainData(RenderableTerrainUPtr renderable) :
				renderable(std::move(renderable)) {};
		};

		struct StepData
		{
			std::shared_ptr<graphics::Step> step;
			std::shared_ptr<graphics::Program> program;
			std::shared_ptr<graphics::UniformVariableValue<glm::mat4>> viewMatrix;
			std::shared_ptr<graphics::UniformVariableValue<glm::mat4>> projectionMatrix;
			std::shared_ptr<graphics::UniformVariableValue<unsigned int>> numPointLights;
			std::shared_ptr<graphics::UniformVariableValueVector<glm::vec3, kMaxPointLights>> pointLightsPositions;
			std::shared_ptr<graphics::UniformBlock> lightsBlock;
		};

		std::map<Entity*, CameraUPtr> cameraEntities;
		std::map<Entity*, LightUPtr> lightEntities;
		std::multimap<Entity*, RenderableMeshData> renderableMeshEntities;
		std::map<Entity*, RenderableTerrainData> renderableTerrainEntities;

		Camera* activeCamera;
		std::shared_ptr<graphics::UniformBuffer> lightsBuffer;
		std::vector<StepData> stepsData;
	};


	GraphicsManager::GraphicsManager(graphics::GraphicsEngine& graphicsEngine, EventManager& eventManager) :
		mGraphicsEngine(graphicsEngine), mEventManager(eventManager)
	{
		mEventManager.subscribe(this, Topic::Resize);

		mImpl = std::make_unique<Impl>();

		// Reserve memory for the UniformBuffers
		mImpl->lightsBuffer = std::make_shared<graphics::UniformBuffer>();
		utils::FixedVector<ShaderPointLight, kMaxPointLights> lightsBufferData(kMaxPointLights);
		mImpl->lightsBuffer->resizeAndCopy(lightsBufferData.data(), lightsBufferData.size());
	}


	GraphicsManager::~GraphicsManager()
	{
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


	void GraphicsManager::addLightEntity(Entity* entity, LightUPtr light)
	{
		if (!entity || !light) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as ILight";
			return;
		}

		// The PointLight initial data is overridden by the entity one
		ILight* lPtr = light.get();
		if (auto dLight = dynamic_cast<DirectionalLight*>(lPtr)) {
			dLight->direction = glm::vec3(0.0f, 0.0f, 1.0f) * entity->orientation;
		}
		else if (auto pLight = dynamic_cast<PointLight*>(lPtr)) {
			pLight->position = entity->position;
		}
		else if (auto sLight = dynamic_cast<SpotLight*>(lPtr)) {
			sLight->position = entity->position;
			sLight->direction = glm::vec3(0.0f, 0.0f, 1.0f) * entity->orientation;
		}

		// Add the ILight
		mImpl->lightEntities.emplace(entity, std::move(light));
		SOMBRA_INFO_LOG << "Entity " << entity << " with ILight " << lPtr << " added successfully";
	}


	GraphicsManager::StepSPtr GraphicsManager::createStep2D(ProgramSPtr program)
	{
		auto step = std::make_unique<graphics::Step2D>(mGraphicsEngine.getRenderer2D());

		step->addBindable(program)
			.addBindable(std::make_shared<graphics::BlendingOperation>(true))
			.addBindable(std::make_shared<graphics::DepthTestOperation>(false));

		for (int i = 0; i < static_cast<int>(graphics::Renderer2D::kMaxTextures); ++i) {
			utils::ArrayStreambuf<char, 64> aStreambuf;
			std::ostream(&aStreambuf) << "uTextures[" << i << "]";
			step->addBindable(std::make_shared<graphics::UniformVariableValue<int>>(aStreambuf.data(), *program, i));
		}

		step->addBindable(std::make_shared<graphics::UniformVariableCallback<glm::mat4>>(
			"uProjectionMatrix", *program,
			[this]() {
				auto viewportSize = mGraphicsEngine.getViewportSize();
				return glm::ortho(0.0f, static_cast<float>(viewportSize.x), static_cast<float>(viewportSize.y), 0.0f, -1.0f, 1.0f);
			}
		));

		return step;
	}


	GraphicsManager::StepSPtr GraphicsManager::createStep3D(ProgramSPtr program, bool hasLights)
	{
		auto step = std::make_shared<graphics::Step3D>(mGraphicsEngine.getRenderer3D());

		auto& stepData = mImpl->stepsData.emplace_back();
		stepData.step = step;
		stepData.program = program;

		stepData.viewMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix", *program);
		stepData.projectionMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", *program);
		if (mImpl->activeCamera) {
			stepData.viewMatrix->setValue(mImpl->activeCamera->getViewMatrix());
			stepData.projectionMatrix->setValue(mImpl->activeCamera->getProjectionMatrix());
		}

		step->addBindable(program)
			.addBindable(stepData.viewMatrix)
			.addBindable(stepData.projectionMatrix);

		if (hasLights) {
			stepData.numPointLights = std::make_shared<graphics::UniformVariableValue<unsigned int>>("uNumPointLights", *program);
			stepData.pointLightsPositions = std::make_shared<graphics::UniformVariableValueVector<glm::vec3, GraphicsManager::kMaxPointLights>>("uPointLightsPositions", *program);
			stepData.lightsBlock = std::make_shared<graphics::UniformBlock>("LightsBlock", *program);

			step->addBindable(mImpl->lightsBuffer)
				.addBindable(stepData.numPointLights)
				.addBindable(stepData.pointLightsPositions)
				.addBindable(stepData.lightsBlock);
		}

		return step;
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

		rPtr->processTechniques([&](auto technique) { technique->processSteps([&](auto step) {
			auto itStepData = std::find_if(mImpl->stepsData.begin(), mImpl->stepsData.end(), [&](const Impl::StepData& stepData) {
				return stepData.step == step;
			});
			if (itStepData != mImpl->stepsData.end()) {
				rPtr->addBindable(
					meshData.modelMatrix.emplace_back(
						std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", *itStepData->program, modelMatrix)
					)
				);

				if (skin) {
					auto jointMatrices = calculateJointMatrices(*skin, modelMatrix);
					std::size_t numJoints = std::min(jointMatrices.size(), static_cast<std::size_t>(kMaxJoints));

					rPtr->addBindable(
						meshData.jointMatrices.emplace_back(
							std::make_shared<graphics::UniformVariableValueVector<glm::mat4, kMaxJoints>>(
								"uJointMatrices", *itStepData->program, jointMatrices.data(), numJoints
							)
						)
					);
				}
			}
			else {
				SOMBRA_WARN_LOG << "RenderableMesh has an Step " << step << " not added to the GraphicsManager";
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

		rPtr->processTechniques([&](auto technique) { technique->processSteps([&](auto step) {
			auto itStepData = std::find_if(mImpl->stepsData.begin(), mImpl->stepsData.end(), [&](const Impl::StepData& stepData) {
				return stepData.step == step;
			});
			if (itStepData != mImpl->stepsData.end()) {
				rPtr->addBindable(
					terrainData.modelMatrix.emplace_back(
						std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", *itStepData->program, modelMatrix)
					)
				);
			}
			else {
				SOMBRA_WARN_LOG << "RenderableTerrain has an Step " << step << " not added to the GraphicsManager";
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

		SOMBRA_DEBUG_LOG << "Updating the ILights";
		bool pointLightsUpdated = false;
		for (auto& pair : mImpl->lightEntities) {
			Entity* entity = pair.first;
			ILight* light = pair.second.get();

			if (entity->updated.any()) {
				if (auto dLight = (dynamic_cast<DirectionalLight*>(light))) {
					dLight->direction = glm::vec3(0.0f, 0.0f, 1.0f) * entity->orientation;
				}
				else if (auto pLight = (dynamic_cast<PointLight*>(light))) {
					pLight->position = entity->position;
					pointLightsUpdated = true;
				}
				else if (auto sLight = (dynamic_cast<SpotLight*>(light))) {
					sLight->position = entity->position;
					sLight->direction = glm::vec3(0.0f, 0.0f, 1.0f) * entity->orientation;
				}
			}
		}

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

		SOMBRA_DEBUG_LOG << "Updating the Steps";
		if (activeCameraUpdated) {
			for (auto& stepData : mImpl->stepsData) {
				stepData.viewMatrix->setValue(mImpl->activeCamera->getViewMatrix());
				stepData.projectionMatrix->setValue(mImpl->activeCamera->getProjectionMatrix());
			}
		}

		if (pointLightsUpdated) {
			unsigned int uNumPointLights = 0;
			utils::FixedVector<ShaderPointLight, kMaxPointLights> uPointLights;
			utils::FixedVector<glm::vec3, kMaxPointLights> uPointLightsPositions;

			for (auto& pair : mImpl->lightEntities) {
				const PointLight* pLight = dynamic_cast<const PointLight*>(pair.second.get());
				if (pLight && (uNumPointLights < kMaxPointLights)) {
					uPointLights.push_back({ pLight->color, pLight->intensity, pLight->inverseRange });
					uPointLightsPositions.push_back(pLight->position);
					uNumPointLights++;
				}
			}

			mImpl->lightsBuffer->copy(uPointLights.data(), uPointLights.size());
			for (auto& stepData : mImpl->stepsData) {
				if (stepData.lightsBlock) {
					stepData.numPointLights->setValue(uNumPointLights);
					stepData.pointLightsPositions->setValue(uPointLightsPositions.data(), uPointLightsPositions.size());
				}
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
		auto width = static_cast<unsigned int>(event.getWidth());
		auto height = static_cast<unsigned int>(event.getHeight());

		mGraphicsEngine.setViewportSize({ width, height });
	}

}
