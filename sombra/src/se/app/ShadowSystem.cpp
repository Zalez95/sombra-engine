#include "se/utils/Log.h"
#include "se/graphics/3D/Renderer3D.h"
#include "se/graphics/3D/FrustumFilter.h"
#include "se/graphics/RenderGraph.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/graphics/ViewportResolutionNode.h"
#include "se/app/ShadowSystem.h"
#include "se/app/Application.h"
#include "se/app/TransformsComponent.h"
#include "se/app/LightComponent.h"
#include "se/app/MeshComponent.h"
#include "se/app/TerrainComponent.h"
#include "se/app/CameraComponent.h"
#include "graphics/ShadowRenderSubGraph.h"
#include "graphics/DeferredLightRenderer.h"

namespace se::app {

	ShadowSystem::ShadowSystem(Application& application) :
		ISystem(application.getEntityDatabase()), mApplication(application)
	{
		mApplication.getEventManager()
			.subscribe(this, Topic::Shadow)
			.subscribe(this, Topic::RMesh)
			.subscribe(this, Topic::RShader)
			.subscribe(this, Topic::Shader);
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask()
			.set<LightComponent>()
			.set<MeshComponent>()
			.set<TerrainComponent>()
		);

		auto& renderGraph = mApplication.getExternalTools().graphicsEngine->getRenderGraph();
		mShadowRenderSubGraph = dynamic_cast<ShadowRenderSubGraph*>(renderGraph.getNode("shadowRenderSubGraph"));
		mDeferredLightRenderer = dynamic_cast<DeferredLightRenderer*>(renderGraph.getNode("deferredLightRenderer"));
	}


	ShadowSystem::~ShadowSystem()
	{
		mEntityDatabase.removeSystem(this);
		mApplication.getEventManager()
			.unsubscribe(this, Topic::Shader)
			.unsubscribe(this, Topic::RShader)
			.unsubscribe(this, Topic::RMesh)
			.unsubscribe(this, Topic::Shadow);
	}


	bool ShadowSystem::notify(const IEvent& event)
	{
		return tryCall(&ShadowSystem::onShadowEvent, event)
			|| tryCall(&ShadowSystem::onRMeshEvent, event)
			|| tryCall(&ShadowSystem::onRenderableShaderEvent, event)
			|| tryCall(&ShadowSystem::onShaderEvent, event);
	}


	void ShadowSystem::onNewComponent(Entity entity, const EntityDatabase::ComponentMask& mask)
	{
		tryCallC(&ShadowSystem::onNewLight, entity, mask);
		tryCallC(&ShadowSystem::onNewMesh, entity, mask);
		tryCallC(&ShadowSystem::onNewTerrain, entity, mask);
	}


	void ShadowSystem::onRemoveComponent(Entity entity, const EntityDatabase::ComponentMask& mask)
	{
		tryCallC(&ShadowSystem::onRemoveLight, entity, mask);
		tryCallC(&ShadowSystem::onRemoveMesh, entity, mask);
		tryCallC(&ShadowSystem::onRemoveTerrain, entity, mask);
	}


	void ShadowSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Renderers";

		CameraComponent camera;
		for (const auto& [entity, shadowIndices] : mShadowEntityMap) {
			auto [transforms, light] = mEntityDatabase.getComponents<TransformsComponent, LightComponent>(entity, true);
			if (transforms && light) {
				camera.setPosition(transforms->position);
				camera.setOrientation(transforms->orientation);

				auto shadowData = light->getShadowData();
				if (light->getSource()->type == LightSource::Type::Point) {
					static const glm::quat kPointLightDirections[] = {
						glm::quat({ 1.0f, 0.0f, 0.0f}), glm::quat(glm::vec3(-1.0f, 0.0f, 0.0f)),
						glm::quat({ 0.0f, 1.0f, 0.0f}), glm::quat(glm::vec3( 0.0f,-1.0f, 0.0f)),
						glm::quat({ 0.0f, 0.0f, 1.0f}), glm::quat(glm::vec3( 0.0f, 0.0f,-1.0f)),
					};

					for (std::size_t i = 0; i < shadowIndices.size(); ++i) {
						std::size_t shadowIndex = shadowIndices[i];
						camera.setOrientation(kPointLightDirections[i]);
						camera.setPerspectiveProjection(glm::radians(45.0f), 1.0f, shadowData->zNear, shadowData->zFar);
						mShadowRenderSubGraph->setShadowVPMatrix(shadowIndex, camera.getViewMatrix(), camera.getProjectionMatrix());
					}
				}
				else {
					for (std::size_t shadowIndex : shadowIndices) {
						// TODO: change znear
						if (light->getSource()->type == LightSource::Type::Directional) {
							camera.setOrthographicProjection(
								-shadowData->size, shadowData->size, -shadowData->size, shadowData->size,
								shadowData->zNear, shadowData->zFar
							);
						}
						else {
							camera.setPerspectiveProjection(
								glm::radians(45.0f), 1.0f,
								shadowData->zNear, shadowData->zFar
							);
						}

						mShadowRenderSubGraph->setShadowVPMatrix(shadowIndex, camera.getViewMatrix(), camera.getProjectionMatrix());
					}
				}
			}
		}

		SOMBRA_INFO_LOG << "Update end";
	}

// Private functions
	void ShadowSystem::onNewLight(Entity entity, LightComponent* light)
	{
		light->setup(&mApplication.getEventManager(), entity);

		std::size_t numNewShadows = !light->getSource()? 0 :
			!light->getShadowData()? 0 :
			(light->getSource()->type == LightSource::Type::Point)? 6 :
			light->getShadowData()->numCascades;
		if (numNewShadows > 0) {
			addShadows(entity, light, numNewShadows);
		}

		SOMBRA_INFO_LOG << "Entity " << entity << " with LightComponent " << light << " added successfully";
	}


	void ShadowSystem::onRemoveLight(Entity entity, LightComponent* light)
	{
		auto itShadows = mShadowEntityMap.find(entity);
		if (itShadows != mShadowEntityMap.end()) {
			removeShadows(entity, light, itShadows->second.size());
		}

		light->setup(nullptr, kNullEntity);

		SOMBRA_INFO_LOG << "Entity " << entity << " with LightComponent " << light << " removed successfully";
	}


	void ShadowSystem::onNewMesh(Entity entity, MeshComponent* mesh)
	{
		mesh->processRenderableIndices([&, mesh = mesh](std::size_t i) {
			mShadowRenderSubGraph->getShadowUniformsUpdater()->addRenderable(mesh->get(i));
			mesh->processRenderableShaders(i, [&](const auto& shader) {
				mShadowRenderSubGraph->getShadowUniformsUpdater()->addRenderableTechnique(mesh->get(i), shader->getTechnique());
			});
		});
		SOMBRA_INFO_LOG << "Entity " << entity << " with MeshComponent " << mesh << " added successfully";
	}


	void ShadowSystem::onRemoveMesh(Entity entity, MeshComponent* mesh)
	{
		mesh->processRenderableIndices([&, mesh = mesh](std::size_t i) {
			mShadowRenderSubGraph->getShadowUniformsUpdater()->removeRenderable(mesh->get(i));
		});
		SOMBRA_INFO_LOG << "Entity " << entity << " with MeshComponent " << mesh << " removed successfully";
	}


	void ShadowSystem::onNewTerrain(Entity entity, TerrainComponent* terrain)
	{
		mShadowRenderSubGraph->getShadowUniformsUpdater()->addRenderable(terrain->get());
		terrain->processRenderableShaders([&](const auto& shader) {
			mShadowRenderSubGraph->getShadowUniformsUpdater()->addRenderableTechnique(terrain->get(), shader->getTechnique());
		});
		SOMBRA_INFO_LOG << "Entity " << entity << " with TerrainComponent " << terrain << " added successfully";
	}


	void ShadowSystem::onRemoveTerrain(Entity entity, TerrainComponent* terrain)
	{
		mShadowRenderSubGraph->getShadowUniformsUpdater()->removeRenderable(terrain->get());
		SOMBRA_INFO_LOG << "Entity " << entity << " with TerrainComponent " << terrain << " removed successfully";
	}


	void ShadowSystem::onShadowEvent(const ContainerEvent<Topic::Shadow, Entity>& event)
	{
		auto [light] = mEntityDatabase.getComponents<LightComponent>(event.getValue(), true);
		if (!light) {
			SOMBRA_WARN_LOG << "Couldn't update the Shadows of the Entity " << event.getValue();
			return;
		}

		int numNewShadows = !light->getSource()? 0 :
			!light->getShadowData()? 0 :
			(light->getSource()->type == LightSource::Type::Point)? 6 :
			static_cast<int>(light->getShadowData()->numCascades);

		auto itShadows = mShadowEntityMap.find(event.getValue());
		int numCurrentShadows = (itShadows != mShadowEntityMap.end())? static_cast<int>(itShadows->second.size()) : 0;

		int diffShadows = numNewShadows - numCurrentShadows;
		if (diffShadows > 0) {
			addShadows(event.getValue(), light, diffShadows);
		}
		else if (diffShadows < 0) {
			removeShadows(event.getValue(), light, -diffShadows);
		}
	}


	void ShadowSystem::onRMeshEvent(const RMeshEvent& event)
	{
		auto [mesh] = mEntityDatabase.getComponents<MeshComponent>(event.getEntity(), true);
		if (mesh) {
			switch (event.getOperation()) {
				case RMeshEvent::Operation::Add:
					mShadowRenderSubGraph->getShadowUniformsUpdater()->addRenderable(mesh->get(event.getRIndex()));
					break;
				case RMeshEvent::Operation::Remove:
					mShadowRenderSubGraph->getShadowUniformsUpdater()->removeRenderable(mesh->get(event.getRIndex()));
					break;
			}
		}
	}


	void ShadowSystem::onRenderableShaderEvent(const RenderableShaderEvent& event)
	{
		if (event.getRComponentType() == RenderableShaderEvent::RComponentType::Mesh) {
			auto [mesh] = mEntityDatabase.getComponents<MeshComponent>(event.getEntity(), true);
			if (mesh) {
				switch (event.getOperation()) {
					case RenderableShaderEvent::Operation::Add:
						mShadowRenderSubGraph->getShadowUniformsUpdater()->addRenderableTechnique(mesh->get(event.getRIndex()), event.getShader()->getTechnique());
						break;
					case RenderableShaderEvent::Operation::Remove:
						mShadowRenderSubGraph->getShadowUniformsUpdater()->removeRenderableTechnique(mesh->get(event.getRIndex()), event.getShader()->getTechnique());
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
						mShadowRenderSubGraph->getShadowUniformsUpdater()->addRenderableTechnique(*renderable, event.getShader()->getTechnique());
						break;
					case RenderableShaderEvent::Operation::Remove:
						mShadowRenderSubGraph->getShadowUniformsUpdater()->removeRenderableTechnique(*renderable, event.getShader()->getTechnique());
						break;
				}
			}
		}
	}


	void ShadowSystem::onShaderEvent(const ShaderEvent& event)
	{
		switch (event.getOperation()) {
			case ShaderEvent::Operation::Add:
				mShadowRenderSubGraph->getShadowUniformsUpdater()->onAddTechniquePass(event.getShader()->getTechnique(), event.getStep()->getPass());
				break;
			case ShaderEvent::Operation::Remove:
				mShadowRenderSubGraph->getShadowUniformsUpdater()->onRemoveTechniquePass(event.getShader()->getTechnique(), event.getStep()->getPass());
				break;
		}
	}


	void ShadowSystem::addShadows(Entity entity, LightComponent* light, std::size_t numShadows)
	{
		auto itShadows = mShadowEntityMap.find(entity);
		if (itShadows == mShadowEntityMap.end()) {
			itShadows = mShadowEntityMap.emplace(entity, std::vector<std::size_t>()).first;
		}

		while (numShadows > 0) {
			auto shadowIndex = mShadowRenderSubGraph->addShadow(light->getShadowData()->resolution);
			if (shadowIndex < MergeShadowsNode::kMaxShadows) {
				itShadows->second.push_back(shadowIndex);
				numShadows--;
			}
			else {
				SOMBRA_WARN_LOG << "Can't add more Shadows to the Entity " << entity;
				break;
			}
		}

		setShadowIndices(entity, light);
	}


	void ShadowSystem::removeShadows(Entity entity, LightComponent* light, std::size_t numShadows)
	{
		auto itShadows = mShadowEntityMap.find(entity);
		if (itShadows == mShadowEntityMap.end()) {
			SOMBRA_WARN_LOG << "Shadows Entity " << entity << " not found";
			return;
		}

		while (numShadows > 0) {
			if (!itShadows->second.empty()) {
				std::size_t shadowIndex = itShadows->second.back();
				itShadows->second.pop_back();
				mShadowRenderSubGraph->removeShadow(shadowIndex);
				numShadows--;
			}
			else {
				SOMBRA_WARN_LOG << "Cant't remove more Shadows from the Entity " << entity;
				break;
			}
		}

		if (itShadows->second.empty()) {
			mShadowEntityMap.erase(itShadows);
		}

		setShadowIndices(entity, light);
	}


	void ShadowSystem::setShadowIndices(Entity entity, LightComponent* light) const
	{
		std::size_t shadowIndices = 0;
		auto itShadows = mShadowEntityMap.find(entity);
		if (itShadows != mShadowEntityMap.end()) {
			for (std::size_t shadowIndex : itShadows->second) {
				shadowIndices = (shadowIndices << 4) | shadowIndex;
			}
		}
		light->setShadowIndices(static_cast<int>(shadowIndices));
	}

}
