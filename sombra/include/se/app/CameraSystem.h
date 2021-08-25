#ifndef CAMERA_SYSTEM_H
#define CAMERA_SYSTEM_H

#include "../graphics/3D/FrustumFilter.h"
#include "events/ContainerEvent.h"
#include "events/RMeshEvent.h"
#include "events/ShaderEvent.h"
#include "events/RenderableShaderEvent.h"
#include "ECS.h"

namespace se::app {

	class Application;
	class CameraComponent;
	class MeshComponent;
	class TerrainComponent;
	class LightComponent;
	class ParticleSystemComponent;
	class SSAONode;
	class DeferredAmbientRenderer;


	/**
	 * Class CameraSystem, it's the System used for updating the Entities'
	 * Cameras, the view and projection matrices on their shaders and the
	 * Renderers that use the Cameras
	 * @note	it will only update the cameras of the Steps that use the
	 *			"forwardRenderer" or the "gBufferRenderer"
	 */
	class CameraSystem : public ISystem
	{
	private:	// Nested Types
		using FrustumFilterSPtr = std::shared_ptr<graphics::FrustumFilter>;
		class CameraUniformsUpdater;

	private:	// Attributes
		/** The Application that holds the Entities */
		Application& mApplication;

		/** The Camera Entity used for rendering */
		Entity mCameraEntity;

		/** The object used for updating the camera uniforms in the Entities
		 * steps */
		CameraUniformsUpdater* mCameraUniformsUpdater;

		/** A pointer to the SSAONode used for computing the ambient
		 * occlussion */
		SSAONode* mSSAONode;

		/** A pointer to the deferred ambient renderer used for computing the
		 * ambient lighting */
		DeferredAmbientRenderer* mDeferredAmbientRenderer;

		/** A pointer to the frustum filter shared between all the
		 * Renderer3Ds */
		FrustumFilterSPtr mFrustumFilter;

	public:		// Functions
		/** Creates a new CameraSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System */
		CameraSystem(Application& application);

		/** Class destructor */
		~CameraSystem();

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

		/** Updates the Cameras sources with the Entities */
		virtual void update() override;
	private:
		/** Function called when a Camera is added to an Entity
		 *
		 * @param	entity the Entity that holds the Camera
		 * @param	camera a pointer to the new Camera */
		void onNewCamera(Entity entity, CameraComponent* camera);

		/** Function called when a Camera is going to be removed from an Entity
		 *
		 * @param	entity the Entity that holds the Camera
		 * @param	camera a pointer to the Camera that is going to be
		 *			removed */
		void onRemoveCamera(Entity entity, CameraComponent* camera);

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

		/** Function called when a ParticleSystemComponent is added to an Entity
		 *
		 * @param	entity the Entity that holds the ParticleSystemComponent
		 * @param	particleSystem a pointer to the new
		 *			ParticleSystemComponent */
		void onNewParticleSys(
			Entity entity, ParticleSystemComponent* particleSystem
		);

		/** Function called when a ParticleSystemComponent is going to be
		 * removed from an Entity
		 *
		 * @param	entity the Entity that holds the ParticleSystemComponent
		 * @param	particleSystem a pointer to the ParticleSystemComponent
		 *			that is going to be removed */
		void onRemoveParticleSys(
			Entity entity, ParticleSystemComponent* particleSystem
		);

		/** Function called when a LightComponent is added to an Entity
		 *
		 * @param	entity the Entity that holds the LightComponent
		 * @param	particleSystem a pointer to the new
		 *			LightComponent */
		void onNewLight(Entity entity, LightComponent* light);

		/** Function called when a LightComponent is going to be
		 * removed from an Entity
		 *
		 * @param	entity the Entity that holds the LightComponent
		 * @param	particleSystem a pointer to the LightComponent
		 *			that is going to be removed */
		void onRemoveLight(Entity entity, LightComponent* light);

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
