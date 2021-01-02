#ifndef CAMERA_SYSTEM_H
#define CAMERA_SYSTEM_H

#include "graphics/FrustumRenderer3D.h"
#include "graphics/DeferredLightRenderer.h"
#include "events/ContainerEvent.h"
#include "events/RMeshEvent.h"
#include "events/ShaderEvent.h"
#include "events/RenderableShaderEvent.h"
#include "ISystem.h"
#include "CameraComponent.h"

namespace se::app {

	class Application;


	/**
	 * Class CameraSystem, it's the System used for updating the Entities'
	 * Cameras, the view and projection matrices on their shaders and the
	 * Renderers that use the Cameras
	 * @note	it will only update the cameras of the Passes that use the
	 *			"forwardRenderer" or the "gBufferRenderer"
	 */
	class CameraSystem : public ISystem
	{
	private:	// Nested Types
		class CameraUniformsUpdater;

	private:	// Attributes
		/** The Application that holds the Entities */
		Application& mApplication;

		/** The active CameraComponent of the Entity */
		CameraComponent* mCamera;

		/** The object used for updating the camera uniforms in the Entities
		 * passes */
		CameraUniformsUpdater* mCameraUniformsUpdater;

		/** A pointer to the forward renderer used for rendering Entities */
		FrustumRenderer3D* mForwardRenderer;

		/** A pointer to the g-buffer renderer used for rendering Entities */
		FrustumRenderer3D* mGBufferRenderer;

		/** A pointer to the deferred light renderer used for computing the
		 * lighting */
		DeferredLightRenderer* mDeferredLightRenderer;

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
		/** Handles the given ContainerEvent by updating the Camera Entity with
		 * which the Scene will be rendered
		 *
		 * @param	event the ContainerEvent to handle */
		void onCameraEvent(const ContainerEvent<Topic::Camera, Entity>& event);

		/** Handles the given RMeshEvent by updating the RenderableMeshes
		 * uniforms
		 *
		 * @param	event the RMeshEvent to handle */
		void onRMeshEvent(const RMeshEvent& event);

		/** Handles the given RenderableShaderEvent by updating the
		 * Renderables uniforms
		 *
		 * @param	event the RenderableShaderEvent to handle */
		void onRenderableShaderEvent(const RenderableShaderEvent& event);

		/** Handles the given ShaderEvent by updating the Renderables uniforms
		 *
		 * @param	event the ShaderEvent to handle */
		void onShaderEvent(const ShaderEvent& event);
	};

}

#endif		// CAMERA_SYSTEM_H
