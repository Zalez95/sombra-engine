#include "GraphicsManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Entity.h"

namespace game {

	void GraphicsManager::addEntity(Entity* entity, CameraUPtr camera)
	{
		if (!entity || !camera) return;

		mGraphicsSystem.getLayer3D()->setCamera(camera.get());
		mCameraEntities.emplace(entity, std::move(camera));
	}


	void GraphicsManager::addEntity(Entity* entity, Renderable3DUPtr renderable3D)
	{
		if (!entity || !renderable3D) return;

		mGraphicsSystem.getLayer3D()->addRenderable3D(renderable3D.get());
		mRenderable3DEntities.emplace(entity, std::move(renderable3D));
	}


	void GraphicsManager::addEntity(Entity* entity, PointLightUPtr pointLight)
	{
		if (!entity || !pointLight) return;

		mGraphicsSystem.getLayer3D()->addPointLight(pointLight.get());
		mPointLightEntities.emplace(entity, std::move(pointLight));
	}


	void GraphicsManager::removeEntity(Entity* entity)
	{
		auto itCamera = mCameraEntities.find(entity);
		if (itCamera != mCameraEntities.end()) {
			mGraphicsSystem.getLayer3D()->setCamera(nullptr);
			mCameraEntities.erase(itCamera);
		}

		auto itRenderable3D = mRenderable3DEntities.find(entity);
		if (itRenderable3D != mRenderable3DEntities.end()) {
			mGraphicsSystem.getLayer3D()->removeRenderable3D(itRenderable3D->second.get());
			mRenderable3DEntities.erase(itRenderable3D);
		}

		auto itPointLight = mPointLightEntities.find(entity);
		if (itPointLight != mPointLightEntities.end()) {
			mGraphicsSystem.getLayer3D()->removePointLight(itPointLight->second.get());
			mPointLightEntities.erase(itPointLight);
		}
	}


	void GraphicsManager::update()
	{
		for (auto& ce : mCameraEntities) {
			glm::vec3 forwardVector = glm::vec3(0, 0,-1) * ce.first->mOrientation;
			glm::vec3 upVector		= glm::vec3(0, 1, 0);

			ce.second->setPosition(ce.first->mPosition);
			ce.second->setTarget(ce.first->mPosition + forwardVector);
			ce.second->setUp(upVector);
		}

		for (auto& pe : mPointLightEntities) {
			pe.second->setPosition(pe.first->mPosition);
		}

		for (auto& re : mRenderable3DEntities) {
			glm::mat4 translation	= glm::translate(glm::mat4(), re.first->mPosition);
			glm::mat4 rotation		= glm::mat4_cast(re.first->mOrientation);
			re.second->setModelMatrix(translation * rotation);
		}
	}


	void GraphicsManager::render()
	{
		mGraphicsSystem.render();
	}

}
