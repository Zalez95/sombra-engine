#include "GraphicsComponent.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Entity.h"

namespace game {

	GraphicsComponent::GraphicsComponent(
		graphics::GraphicsSystem& graphicsSystem,
		std::unique_ptr<graphics::Camera> camera,
		std::unique_ptr<graphics::PointLight> pointLight,
		std::unique_ptr<graphics::Renderable3D> renderable3D
	) : mGraphicsSystem(graphicsSystem),
		mCamera(std::move(camera)), mPointLight(std::move(pointLight)),
		mRenderable3D(std::move(renderable3D))
	{
		if (mCamera)
			mGraphicsSystem.getLayer3D()->setCamera(mCamera.get());
		if (mPointLight)
			mGraphicsSystem.getLayer3D()->addPointLight(mPointLight.get());
		if (mRenderable3D)
			mGraphicsSystem.getLayer3D()->addRenderable3D(mRenderable3D.get());
	}


	GraphicsComponent::~GraphicsComponent()
	{
		if (mPointLight)
			mGraphicsSystem.getLayer3D()->removePointLight(mPointLight.get());
		if (mRenderable3D)
			mGraphicsSystem.getLayer3D()->removeRenderable3D(mRenderable3D.get());
	}


	void GraphicsComponent::update(Entity& entity, float delta)
	{
		if (mCamera) {
			glm::vec3 forwardVector = glm::vec3(0, 0, -1) * entity.mOrientation;
			glm::vec3 upVector		= glm::vec3(0, 1, 0);
				
			mCamera->setPosition(entity.mPosition);
			mCamera->setTarget(entity.mPosition + forwardVector);
			mCamera->setUp(upVector);
		}
		if (mPointLight) {
			mPointLight->setPosition(entity.mPosition);
		}
		if (mRenderable3D) {
			glm::mat4 translation = glm::translate(glm::mat4(), entity.mPosition);
			glm::mat4 rotation = glm::mat4_cast(entity.mOrientation);
			mRenderable3D->setModelMatrix(translation * rotation);
		}
	}

}
