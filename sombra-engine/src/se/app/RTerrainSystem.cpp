#include "se/utils/Log.h"
#include "se/graphics/Technique.h"
#include "se/graphics/Pass.h"
#include "se/app/RTerrainSystem.h"
#include "se/app/EntityDatabase.h"
#include "se/app/TransformsComponent.h"
#include "se/app/graphics/Camera.h"

namespace se::app {

	RTerrainSystem::RTerrainSystem(
		EntityDatabase& entityDatabase, graphics::GraphicsEngine& graphicsEngine, CameraSystem& cameraSystem
	) : ISystem(entityDatabase), mGraphicsEngine(graphicsEngine), mCameraSystem(cameraSystem)
	{
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask().set<graphics::RenderableTerrain>());
	}


	RTerrainSystem::~RTerrainSystem()
	{
		mEntityDatabase.removeSystem(this);
	}


	void RTerrainSystem::onNewEntity(Entity entity)
	{
		auto [transforms, rTerrain] = mEntityDatabase.getComponents<TransformsComponent, graphics::RenderableTerrain>(entity);
		if (!rTerrain) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as Terrain";
			return;
		}

		glm::mat4 modelMatrix(1.0f);
		if (transforms) {
			glm::mat4 translation	= glm::translate(glm::mat4(1.0f), transforms->position);
			glm::mat4 rotation		= glm::mat4_cast(transforms->orientation);
			modelMatrix = translation * rotation;
		}

		auto& meshData = mRenderableTerrainEntities[entity];
		rTerrain->processTechniques([&, rTerrain = rTerrain](auto technique) { technique->processPasses([&](auto pass) {
			auto it = std::find_if(
				mCameraSystem.mPassesData.begin(), mCameraSystem.mPassesData.end(),
				[&](const auto& passData) { return passData.pass == pass; }
			);
			if (it != mCameraSystem.mPassesData.end()) {
				meshData.modelMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", *it->program, modelMatrix);
				rTerrain->addBindable(meshData.modelMatrix);
			}
			else {
				SOMBRA_WARN_LOG << "RenderableTerrain has a Pass " << pass << " not added to the CameraSystem";
			}
		}); });

		if (mCameraSystem.getActiveCamera()) {
			rTerrain->setHighestLodLocation(mCameraSystem.getActiveCamera()->getPosition());
		}

		mGraphicsEngine.addRenderable(rTerrain);
		SOMBRA_INFO_LOG << "Entity " << entity << " with RenderableTerrain " << rTerrain << " added successfully";
	}


	void RTerrainSystem::onRemoveEntity(Entity entity)
	{
		auto [rTerrain] = mEntityDatabase.getComponents<graphics::RenderableTerrain>(entity);
		if (!rTerrain) {
			SOMBRA_INFO_LOG << "Terrain Entity " << entity << " couldn't removed";
			return;
		}

		auto it = mRenderableTerrainEntities.find(entity);
		if (it != mRenderableTerrainEntities.end()) {
			mRenderableTerrainEntities.erase(it);
		}

		mGraphicsEngine.removeRenderable(rTerrain);
		SOMBRA_INFO_LOG << "Terrain Entity " << entity << " removed successfully";
	}


	void RTerrainSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Terrains";

		mEntityDatabase.iterateComponents<TransformsComponent, graphics::RenderableTerrain>(
			[&](Entity entity, TransformsComponent* transforms, graphics::RenderableTerrain* rTerrain) {
				if (transforms->updated.any()) {
					auto& terrainData = mRenderableTerrainEntities[entity];

					glm::mat4 translation	= glm::translate(glm::mat4(1.0f), transforms->position);
					glm::mat4 rotation		= glm::mat4_cast(transforms->orientation);
					glm::mat4 modelMatrix	= translation * rotation;

					terrainData.modelMatrix->setValue(modelMatrix);
				}

				if (mCameraSystem.getActiveCamera() && mCameraSystem.wasCameraUpdated()) {
					rTerrain->setHighestLodLocation(mCameraSystem.getActiveCamera()->getPosition());
				}
			}
		);

		SOMBRA_INFO_LOG << "Update end";
	}

}
