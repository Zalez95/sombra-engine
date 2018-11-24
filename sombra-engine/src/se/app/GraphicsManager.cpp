#include <glm/gtc/matrix_transform.hpp>
#include "se/app/GraphicsManager.h"
#include "se/app/Entity.h"

namespace se::app {

	GraphicsManager::GraphicsManager(graphics::GraphicsSystem& graphicsSystem) :
		mGraphicsSystem(graphicsSystem)
	{
		mGraphicsSystem.addLayer(&mLayer3D);
	}


	void GraphicsManager::addEntity(Entity* entity, CameraUPtr camera)
	{
		if (!entity || !camera) return;

		mLayer3D.setCamera(camera.get());
		mCameraEntities.emplace(entity, std::move(camera));
	}


	void GraphicsManager::addEntity(Entity* entity, Renderable3DUPtr renderable3D, const glm::mat4& offset)
	{
		if (!entity || !renderable3D) return;

		mLayer3D.addRenderable3D(renderable3D.get());
		mRenderable3DEntities.emplace(entity, std::pair(std::move(renderable3D), offset));
	}


	void GraphicsManager::addEntity(Entity* entity, PointLightUPtr pointLight)
	{
		if (!entity || !pointLight) return;

		mLayer3D.addPointLight(pointLight.get());
		mPointLightEntities.emplace(entity, std::move(pointLight));
	}


	void GraphicsManager::removeEntity(Entity* entity)
	{
		auto itCamera = mCameraEntities.find(entity);
		if (itCamera != mCameraEntities.end()) {
			mLayer3D.setCamera(nullptr);
			mCameraEntities.erase(itCamera);
		}

		auto itRenderable3D = mRenderable3DEntities.find(entity);
		if (itRenderable3D != mRenderable3DEntities.end()) {
			mLayer3D.removeRenderable3D(itRenderable3D->second.first.get());
			mRenderable3DEntities.erase(itRenderable3D);
		}

		auto itPointLight = mPointLightEntities.find(entity);
		if (itPointLight != mPointLightEntities.end()) {
			mLayer3D.removePointLight(itPointLight->second.get());
			mPointLightEntities.erase(itPointLight);
		}
	}


	void GraphicsManager::update()
	{
		for (auto& ce : mCameraEntities) {
			glm::vec3 forwardVector = glm::vec3(0, 0,-1) * ce.first->orientation;
			glm::vec3 upVector		= glm::vec3(0, 1, 0);

			ce.second->setPosition(ce.first->position);
			ce.second->setTarget(ce.first->position + forwardVector);
			ce.second->setUp(upVector);
		}

		for (auto& pe : mPointLightEntities) {
			pe.second->setPosition(pe.first->position);
		}

		for (auto& re : mRenderable3DEntities) {
			glm::mat4 translation	= glm::translate(glm::mat4(1.0f), re.first->position);
			glm::mat4 rotation		= glm::mat4_cast(re.first->orientation);
			glm::mat4 offset		= re.second.second;
			re.second.first->setModelMatrix(offset * translation * rotation);
		}
	}


	void GraphicsManager::render()
	{
		mGraphicsSystem.render();
	}

}
