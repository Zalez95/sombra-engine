#ifndef SHADOW_SYSTEM_H
#define SHADOW_SYSTEM_H

#include "graphics/DeferredLightRenderer.h"
#include "events/ContainerEvent.h"
#include "events/RMeshEvent.h"
#include "events/ShaderEvent.h"
#include "events/RenderableShaderEvent.h"
#include "LightComponent.h"
#include "ECS.h"

namespace se::app {

	class Application;
	class MeshComponent;
	class TerrainComponent;


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

		/** @copydoc ISystem::notify(const IEvent&) */
		virtual void notify(const IEvent& event) override;

		/** @copydoc ISystem::onNewComponent(Entity, const EntityDatabase::ComponentMask&) */
		virtual void onNewComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask
		) override;

		/** @copydoc ISystem::onRemoveComponent(Entity, const EntityDatabase::ComponentMask&) */
		virtual void onRemoveComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask
		) override;

		/** Updates the LightSources with the Entities */
		virtual void update() override;
	private:
		/** Function called when a LightComponent is added to an Entity
		 *
		 * @param	entity the Entity that holds the LightComponent
		 * @param	light a pointer to the new LightComponent */
		void onNewLight(Entity entity, LightComponent* light);

		/** Function called when a LightComponent is going to be removed from
		 * an Entity
		 *
		 * @param	entity the Entity that holds the LightComponent
		 * @param	light a pointer to the LightComponent that is going to be
		 *			removed */
		void onRemoveLight(Entity entity, LightComponent* light);

		/** Function called when a MeshComponent is added to an Entity
		 *
		 * @param	entity the Entity that holds the MeshComponent
		 * @param	mesh a pointer to the new MeshComponent */
		void onNewMesh(Entity entity, MeshComponent* mesh);

		/** Function called when a MeshComponent is going to be removed from an
		 * Entity
		 *
		 * @param	entity the Entity that holds the MeshComponent
		 * @param	mesh a pointer to the MeshComponent that is going to be
		 *			removed */
		void onRemoveMesh(Entity entity, MeshComponent* mesh);

		/** Function called when a TerrainComponent is added to an Entity
		 *
		 * @param	entity the Entity that holds the TerrainComponent
		 * @param	terrain a pointer to the new TerrainComponent */
		void onNewTerrain(Entity entity, TerrainComponent* terrain);

		/** Function called when a TerrainComponent is going to be removed
		 * from an Entity
		 *
		 * @param	entity the Entity that holds the TerrainComponent
		 * @param	terrain a pointer to the TerrainComponent that is going to
		 *			be removed */
		void onRemoveTerrain(Entity entity, TerrainComponent* terrain);

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
