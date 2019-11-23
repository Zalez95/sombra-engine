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


	void GraphicsManager::addCameraEntity(Entity* entity, CameraUPtr camera)
	{
		if (!entity || !camera) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as Camera";
			return;
		}

		// The Camera initial data is overridden by the entity one
		graphics::Camera* cPtr = camera.get();
		cPtr->setPosition(entity->position);
		cPtr->setTarget(entity->position + glm::vec3(0.0f, 0.0f, -1.0f) * entity->orientation);
		cPtr->setUp({ 0.0f, 1.0f, 0.0f });

		// Add the Camera
		mLayer3D.setCamera(cPtr);
		mCameraEntities.emplace(entity, std::move(camera));
		SOMBRA_INFO_LOG << "Entity " << entity << " with Camera " << cPtr << " added successfully";
	}


	void GraphicsManager::addRenderableEntity(Entity* entity, Renderable3DUPtr renderable3D, SkinSPtr skin)
	{
		if (!entity || !renderable3D) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as Renderable3D";
			return;
		}

		// The Renderable3D initial data is overridden by the entity one
		graphics::Renderable3D* rPtr = renderable3D.get();
		glm::mat4 translation	= glm::translate(glm::mat4(1.0f), entity->position);
		glm::mat4 rotation		= glm::mat4_cast(entity->orientation);
		glm::mat4 scale			= glm::scale(glm::mat4(1.0f), entity->scale);
		rPtr->setModelMatrix(translation * rotation * scale);

		// Add the Renderable3D
		mLayer3D.addRenderable3D(rPtr);
		mRenderable3DEntities.emplace(entity, std::move(renderable3D));
		if (skin) {
			mRenderable3DSkins.emplace(rPtr, skin);
			SOMBRA_INFO_LOG << "Entity " << entity << " with Renderable3D " << rPtr << " and skin " << skin.get() << " added successfully";
		}
		else {
			SOMBRA_INFO_LOG << "Entity " << entity << " with Renderable3D " << rPtr << " added successfully";
		}
	}


	void GraphicsManager::addSkyEntity(Entity* entity, Renderable3DUPtr renderable3D)
	{
		if (!entity || !renderable3D) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as Sky Renderable3D";
			return;
		}

		// The Renderable3D initial data is overridden by the entity one
		graphics::Renderable3D* rPtr = renderable3D.get();
		glm::mat4 translation	= glm::translate(glm::mat4(1.0f), entity->position);
		glm::mat4 rotation		= glm::mat4_cast(entity->orientation);
		glm::mat4 scale			= glm::scale(glm::mat4(1.0f), entity->scale);
		rPtr->setModelMatrix(translation * rotation * scale);

		// Add the Renderable3D
		mLayer3D.setSky(rPtr);
		mRenderable3DEntities.emplace(entity, std::move(renderable3D));
		SOMBRA_INFO_LOG << "Entity " << entity << " with Sky Renderable3D " << rPtr << " added successfully";
	}


	void GraphicsManager::addPointLightEntity(Entity* entity, PointLightUPtr pointLight)
	{
		if (!entity || !pointLight) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as PointLight";
			return;
		}

		// The PointLight initial data is overridden by the entity one
		graphics::PointLight* pPtr = pointLight.get();
		pPtr->position = entity->position;

		// Add the PointLight
		mLayer3D.addPointLight(pPtr);
		mPointLightEntities.emplace(entity, std::move(pointLight));
		SOMBRA_INFO_LOG << "Entity " << entity << " with PointLight " << pPtr << " added successfully";
	}


	void GraphicsManager::removeEntity(Entity* entity)
	{
		auto itCamera = mCameraEntities.find(entity);
		if (itCamera != mCameraEntities.end()) {
			mLayer3D.setCamera(nullptr);
			mCameraEntities.erase(itCamera);
			SOMBRA_INFO_LOG << "Camera Entity " << entity << " removed successfully";
		}

		auto itRenderable3D = mRenderable3DEntities.find(entity);
		if (itRenderable3D != mRenderable3DEntities.end()) {
			mLayer3D.removeRenderable3D(itRenderable3D->second.get());
			mRenderable3DEntities.erase(itRenderable3D);
			SOMBRA_INFO_LOG << "Renderable3D Entity " << entity << " removed successfully";
		}

		auto itPointLight = mPointLightEntities.find(entity);
		if (itPointLight != mPointLightEntities.end()) {
			mLayer3D.removePointLight(itPointLight->second.get());
			mPointLightEntities.erase(itPointLight);
			SOMBRA_INFO_LOG << "PointLight Entity " << entity << " removed successfully";
		}
	}


	void GraphicsManager::update()
	{
		SOMBRA_INFO_LOG << "Update start";

		SOMBRA_DEBUG_LOG << "Updating the Cameras";
		for (auto& pair : mCameraEntities) {
			Entity* entity = pair.first;
			graphics::Camera* camera = pair.second.get();

			if (entity->updated.any()) {
				camera->setPosition(entity->position);
				camera->setTarget(entity->position + glm::vec3(0.0f, 0.0f, -1.0f) * entity->orientation);
				camera->setUp({ 0.0f, 1.0f, 0.0f });
			}
		}

		SOMBRA_DEBUG_LOG << "Updating the Renderable3Ds";
		for (auto& pair : mRenderable3DEntities) {
			Entity* entity = pair.first;
			graphics::Renderable3D* renderable3D = pair.second.get();

			if (entity->updated.any()) {
				glm::mat4 translation	= glm::translate(glm::mat4(1.0f), entity->position);
				glm::mat4 rotation		= glm::mat4_cast(entity->orientation);
				glm::mat4 scale			= glm::scale(glm::mat4(1.0f), entity->scale);
				renderable3D->setModelMatrix(translation * rotation * scale);
			}

			// Set the joint matrices of the skeletal animation
			auto itSkin = mRenderable3DSkins.find(renderable3D);
			if (itSkin != mRenderable3DSkins.end()) {
				const Skin& skin = *itSkin->second;
				renderable3D->setJointMatrices( calculateJointMatrices(skin, renderable3D->getModelMatrix()) );
			}
		}

		SOMBRA_DEBUG_LOG << "Updating the PointLights";
		for (auto& pair : mPointLightEntities) {
			Entity* entity = pair.first;
			graphics::PointLight* pointLight = pair.second.get();

			if (entity->updated.any()) {
				pointLight->position = entity->position;
			}
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
