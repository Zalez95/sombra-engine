#ifndef SHADOW_SYSTEM_H
#define SHADOW_SYSTEM_H

#include "events/ContainerEvent.h"
#include "events/RMeshEvent.h"
#include "events/ShaderEvent.h"
#include "events/RenderableShaderEvent.h"
#include "ECS.h"

namespace se::app {

	class Application;
	class LightComponent;
	class MeshComponent;
	class TerrainComponent;
	class ShadowRenderSubGraph;
	class DeferredLightRenderer;


	/**
	 * Class CameraSystem, it's the System used for updating the Entities'
	 * view and projection matrices on their shadow shaders and the shadow
	 * Renderers
	 * @note	it will only update the uniforms of the Steps that use the
	 *			"shadowRenderer"
	 */
	class ShadowSystem : public ISystem
	{
	private:	// Attributes
		/** The Application that holds the Entities */
		Application& mApplication;

		/** A pointer to the ShadowRenderSubGraph used for rendering the
		 * Shadows */
		ShadowRenderSubGraph* mShadowRenderSubGraph;

		/** A pointer to the deferred light renderer used for computing the
		 * lighting */
		DeferredLightRenderer* mDeferredLightRenderer;

		/** Maps each Entity with its shadow indices */
		std::unordered_map<Entity, std::vector<std::size_t>> mShadowEntityMap;

	public:		// Functions
		/** Creates a new ShadowSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System */
		ShadowSystem(Application& application);

		/** Class destructor */
		~ShadowSystem();

		/** @copydoc ISystem::notify(const IEvent&) */
		virtual bool notify(const IEvent& event) override;

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

		/** Adds the given number of shadows to the given Entity
		 *
		 * @param	entity the Entity that will hold the Shadows
		 * @param	light a pointer to the LightComponent of the Entity
		 * @param	numShadows the number of shadows to add */
		void addShadows(
			Entity entity, LightComponent* light, std::size_t numShadows
		);

		/** Removes the given number of shadows from the given Entity
		 *
		 * @param	entity the Entity that holds the Shadows
		 * @param	light a pointer to the LightComponent of the Entity
		 * @param	numShadows the number of shadows to remove */
		void removeShadows(
			Entity entity, LightComponent* light, std::size_t numShadows
		);

		/** Calculates the ShadowIndices of the given Entity
		 *
		 * @param	entity the Entity that holds the LightComponent to
		 *			calculate its shadow indices value
		 * @param	light a pointer to the LightComponent of the Entity */
		void setShadowIndices(Entity entity, LightComponent* light) const;
	};

}

#endif		// SHADOW_SYSTEM_H
