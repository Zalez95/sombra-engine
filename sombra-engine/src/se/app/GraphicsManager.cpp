#include <glm/gtc/matrix_transform.hpp>
#include "se/app/GraphicsManager.h"
#include "se/app/Entity.h"
#include "se/utils/Log.h"

namespace se::app {

	GraphicsManager::GraphicsManager(graphics::GraphicsSystem& graphicsSystem) :
		mGraphicsSystem(graphicsSystem)
	{
		mGraphicsSystem.addLayer(&mLayer3D);
	}


	void GraphicsManager::addEntity(Entity* entity, CameraUPtr camera)
	{
		if (!entity || !camera) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as Camera";
			return;
		}

		mLayer3D.setCamera(camera.get());
		mCameraEntities.emplace(entity, std::move(camera));
		SOMBRA_INFO_LOG << "Entity " << entity << " added successfully as Camera";
	}


	void GraphicsManager::addEntity(Entity* entity, Renderable3DUPtr renderable3D, const glm::mat4& offset)
	{
		if (!entity || !renderable3D) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as Renderable3D";
			return;
		}

		mLayer3D.addRenderable3D(renderable3D.get());
		mRenderable3DEntities.emplace(entity, std::pair(std::move(renderable3D), offset));
		SOMBRA_INFO_LOG << "Entity " << entity << " added successfully as Renderable3D";
	}


	void GraphicsManager::addEntity(Entity* entity, PointLightUPtr pointLight)
	{
		if (!entity || !pointLight) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as PointLight";
			return;
		}

		mLayer3D.addPointLight(pointLight.get());
		mPointLightEntities.emplace(entity, std::move(pointLight));
		SOMBRA_INFO_LOG << "Entity " << entity << " added successfully as PointLight";
	}


	void GraphicsManager::removeEntity(Entity* entity)
	{
		auto itCamera = mCameraEntities.find(entity);
		if (itCamera != mCameraEntities.end()) {
			mLayer3D.setCamera(nullptr);
			mCameraEntities.erase(itCamera);
			SOMBRA_INFO_LOG << "Entity " << entity << " removed successfully as Camera";
		}

		auto itRenderable3D = mRenderable3DEntities.find(entity);
		if (itRenderable3D != mRenderable3DEntities.end()) {
			mLayer3D.removeRenderable3D(itRenderable3D->second.first.get());
			mRenderable3DEntities.erase(itRenderable3D);
			SOMBRA_INFO_LOG << "Entity " << entity << " removed successfully as Renderable3D";
		}

		auto itPointLight = mPointLightEntities.find(entity);
		if (itPointLight != mPointLightEntities.end()) {
			mLayer3D.removePointLight(itPointLight->second.get());
			mPointLightEntities.erase(itPointLight);
			SOMBRA_INFO_LOG << "Entity " << entity << " removed successfully as PointLight";
		}
	}


	void GraphicsManager::update()
	{
		SOMBRA_INFO_LOG << "Update start";

		SOMBRA_INFO_LOG << "Updating Cameras";
		for (auto& ce : mCameraEntities) {
			glm::vec3 forwardVector = glm::vec3(0, 0,-1) * ce.first->orientation;
			glm::vec3 upVector		= glm::vec3(0, 1, 0);

			ce.second->setPosition(ce.first->position);
			ce.second->setTarget(ce.first->position + forwardVector);
			ce.second->setUp(upVector);
		}

		SOMBRA_INFO_LOG << "Updating Renderable3Ds";
		for (auto& re : mRenderable3DEntities) {
			glm::mat4 translation	= glm::translate(glm::mat4(1.0f), re.first->position);
			glm::mat4 rotation		= glm::mat4_cast(re.first->orientation);
			glm::mat4 scale			= glm::scale(glm::mat4(1.0f), re.first->scale);
			glm::mat4 offset		= re.second.second;
			re.second.first->setModelMatrix(offset * translation * rotation * scale);
		}

		SOMBRA_INFO_LOG << "Updating PointLights";
		for (auto& pe : mPointLightEntities) {
			pe.second->setPosition(pe.first->position);
		}

		SOMBRA_INFO_LOG << "Update end";
	}


	void GraphicsManager::render()
	{
		SOMBRA_INFO_LOG << "Render start";
		mGraphicsSystem.render();
		SOMBRA_INFO_LOG << "Render end";
	}

}
