#ifndef CAMERA_SYSTEM_H
#define CAMERA_SYSTEM_H

#include "IVPSystem.h"
#include "graphics/FrustumRenderer3D.h"
#include "events/ContainerEvent.h"
#include "CameraComponent.h"
#include "IVPSystem.h"

namespace se::app {

	/**
	 * Class CameraSystem, it's the System used for updating the Entities'
	 * Cameras, the view and projection matrices on their shaders and the
	 * Renderers that use the Cameras
	 * @note	it will only update the cameras of the Passes that use the
	 *			"forwardRenderer" or the "gBufferRenderer"
	 */
	class CameraSystem : public IVPSystem
	{
	private:	// Attributes
		/** The Entity that holds the current active camera with which the
		 * scene will be rendered */
		Entity mCameraEntity;

		/** The active CameraComponent of the Entity */
		CameraComponent* mCamera;

		/** A pointer to the forward renderer used for rendering Entities */
		FrustumRenderer3D* mForwardRenderer;

		/** A pointer to the g-buffer renderer used for rendering Entities */
		FrustumRenderer3D* mGBufferRenderer;

	public:		// Functions
		/** Creates a new CameraSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System */
		CameraSystem(Application& application);

		/** Class destructor */
		~CameraSystem();

		/** Notifies the CameraSystem of the given event
		 *
		 * @param	event the IEvent to notify */
		virtual void notify(const IEvent& event) override;

		/** @copydoc IVPSystem::onNewEntity(Entity) */
		virtual void onNewEntity(Entity entity) override;

		/** @copydoc IVPSystem::onRemoveEntity(Entity) */
		virtual void onRemoveEntity(Entity entity) override;

		/** Updates the Cameras sources with the Entities */
		virtual void update() override;
	private:
		/** @copydoc IVPSystem::getViewMatrix() */
		virtual glm::mat4 getViewMatrix() const override;

		/** @copydoc IVPSystem::getProjectionMatrix() */
		virtual glm::mat4 getProjectionMatrix() const override;

		/** @copydoc IVPSystem::shouldAddUniforms(PassSPtr) */
		virtual bool shouldAddUniforms(PassSPtr pass) const override;

		/** Handles the given ContainerEvent by updating the Camera Entity with
		 * which the Scene will be rendered
		 *
		 * @param	event the ContainerEvent to handle */
		void onCameraEvent(const ContainerEvent<Topic::Camera, Entity>& event);
	};

}

#endif		// CAMERA_SYSTEM_H
