#ifndef SHADOW_SYSTEM_H
#define SHADOW_SYSTEM_H

#include "graphics/DeferredLightRenderer.h"
#include "events/ContainerEvent.h"
#include "events/RMeshEvent.h"
#include "events/ShaderEvent.h"
#include "events/RenderableShaderEvent.h"
#include "LightComponent.h"
#include "ISystem.h"

namespace se::app {

	class Application;


	/**
	 * Class CameraSystem, it's the System used for updating the Entities'
	 * view and projection matrices on their shadow shaders and the shadow
	 * Renderers
	 * @note	it will only update the uniforms of the Passes that use the
	 *			"shadowRenderer"
	 */
	class ShadowSystem : public ISystem
	{
	private:	// Nested Types
		struct Shadow;
		class ShadowUniformsUpdater;

	private:	// Attributes
		/** The Application that holds the Entities */
		Application& mApplication;

		/** The Entity that holds the current light that creates the shadows */
		Entity mShadowEntity;

		/** The objects used for rendering each the shadow */
		std::vector<Shadow> mShadows;

		/** A pointer to the deferred light renderer used for computing the
		 * lighting */
		DeferredLightRenderer* mDeferredLightRenderer;

	public:		// Functions
		/** Creates a new ShadowSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System
		 * @param	shadowData the configuration used for rendering the
		 *			Shadows */
		ShadowSystem(Application& application, const ShadowData& shadowData);

		/** Class destructor */
		~ShadowSystem();

		/** Notifies the ShadowSystem of the given event
		 *
		 * @param	event the IEvent to notify */
		virtual void notify(const IEvent& event) override;

		/** @copydoc IVPSystem::onNewEntity(Entity) */
		virtual void onNewEntity(Entity entity) override;

		/** @copydoc IVPSystem::onRemoveEntity(Entity) */
		virtual void onRemoveEntity(Entity entity) override;

		/** Updates the LightSources with the Entities */
		virtual void update() override;
	private:
		/** Handles the given ContainerEvent by updating the LightComponent
		 * Entity with which the shadows will be rendered
		 *
		 * @param	event the ContainerEvent to handle */
		void onShadowEvent(const ContainerEvent<Topic::Shadow, Entity>& event);

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

#endif		// SHADOW_SYSTEM_H
