#ifndef LIGHT_SYSTEM_H
#define LIGHT_SYSTEM_H

#include <memory>
#include <unordered_map>
#include "events/ContainerEvent.h"
#include "events/RMeshEvent.h"
#include "events/LightSourceEvent.h"
#include "events/ShaderEvent.h"
#include "events/RenderableShaderEvent.h"
#include "events/EventManager.h"
#include "graphics/TypeRefs.h"
#include "LightComponent.h"
#include "ECS.h"

namespace se::app {

	class Application;
	class MeshComponent;
	class TerrainComponent;
	class ShadowRenderSubGraph;


	/**
	 * Class LightSystem, it's the System used for updating the Entities'
	 * LightComponents
	 */
	class LightSystem : public ISystem, public IEventListener
	{
	private:	// Nested types
		struct EntityUniforms
		{
			std::size_t iDL;
			UniformVVRef<glm::mat4> modelMatrices[2];
			UniformVVRef<unsigned int> type;
			UniformVVRef<glm::vec3> color;
			UniformVVRef<float> intensity;
			UniformVVRef<float> range;
			UniformVVRef<float> lightAngleScale;
			UniformVVRef<float> lightAngleOffset;
			UniformVVVRef<glm::mat4> shadowVPMatrices;
			UniformVVRef<unsigned int> numCascades;
			UniformVVVRef<float> cascadesZFar;
		};

		struct LightVolumeData;

	private:	// Attributes
		/** The Application that holds the Entities */
		Application& mApplication;

		/** The factor used for splitting the CSM frustums */
		float mShadowSplitLogFactor;

		/** A pointer to the ShadowRenderSubGraph used for rendering the
		 * Shadows */
		ShadowRenderSubGraph* mShadowRenderSubGraph = nullptr;

		/** The data used for rendering the light volumes */
		std::unique_ptr<LightVolumeData> mLightVolumeData;

		/** All the uniforms to update for each Entity */
		std::unordered_map<Entity, EntityUniforms> mEntityUniforms;

		/** The camera Entity used for rendering */
		Entity mCameraEntity = kNullEntity;

		/** The mutex that protects @see mEntityUniforms and
		 * @see mCameraEntity */
		std::mutex mMutex;

	public:		// Functions
		/** Creates a new LightSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System
		 * @param	shadowSplitLogFactor the factor used for splitting the CSM
		 *			frustums */
		LightSystem(Application& application, float shadowSplitLogFactor);

		/** Class destructor */
		~LightSystem();

		/** @copydoc IEventListener::notify(const IEvent&) */
		virtual bool notify(const IEvent& event) override;

		/** @copydoc ISystem::onNewComponent(Entity, const EntityDatabase::ComponentMask&, EntityDatabase::Query&) */
		virtual void onNewComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask,
			EntityDatabase::Query& query
		) override;

		/** @copydoc ISystem::onRemoveComponent(Entity, const EntityDatabase::ComponentMask&, EntityDatabase::Query&) */
		virtual void onRemoveComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask,
			EntityDatabase::Query& query
		) override;

		/** Updates the LightSources with the Entities */
		virtual void update() override;
	private:
		/** Function called when a LightComponent is added to an Entity
		 *
		 * @param	entity the Entity that holds the LightComponent
		 * @param	light a pointer to the new LightComponent
		 * @param	query the Query object used for interacting with the Entity
		 *			and its other Components */
		void onNewLight(
			Entity entity, LightComponent* light, EntityDatabase::Query& query
		);

		/** Function called when a LightComponent is going to be removed from
		 * an Entity
		 *
		 * @param	entity the Entity that holds the LightComponent
		 * @param	light a pointer to the LightComponent that is going to be
		 *			removed
		 * @param	query the Query object used for interacting with the Entity
		 *			and its other Components */
		void onRemoveLight(
			Entity entity, LightComponent* light, EntityDatabase::Query& query
		);

		/** Function called when a MeshComponent is added to an Entity
		 *
		 * @param	entity the Entity that holds the MeshComponent
		 * @param	mesh a pointer to the new MeshComponent
		 * @param	query the Query object used for interacting with the Entity
		 *			and its other Components */
		void onNewMesh(
			Entity entity, MeshComponent* mesh, EntityDatabase::Query& query
		);

		/** Function called when a MeshComponent is going to be removed from an
		 * Entity
		 *
		 * @param	entity the Entity that holds the MeshComponent
		 * @param	mesh a pointer to the MeshComponent that is going to be
		 *			removed
		 * @param	query the Query object used for interacting with the Entity
		 *			and its other Components */
		void onRemoveMesh(
			Entity entity, MeshComponent* mesh, EntityDatabase::Query& query
		);

		/** Function called when a TerrainComponent is added to an Entity
		 *
		 * @param	entity the Entity that holds the TerrainComponent
		 * @param	terrain a pointer to the new TerrainComponent
		 * @param	query the Query object used for interacting with the Entity
		 *			and its other Components */
		void onNewTerrain(
			Entity entity, TerrainComponent* terrain,
			EntityDatabase::Query& query
		);

		/** Function called when a TerrainComponent is going to be removed
		 * from an Entity
		 *
		 * @param	entity the Entity that holds the TerrainComponent
		 * @param	terrain a pointer to the TerrainComponent that is going to
		 *			be removed
		 * @param	query the Query object used for interacting with the Entity
		 *			and its other Components */
		void onRemoveTerrain(
			Entity entity, TerrainComponent* terrain,
			EntityDatabase::Query& query
		);

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

		/** Handles the given LightSourceEvent by updating the LightComponents
		 * affected
		 *
		 * @param	event the LightSourceEvent to handle */
		void onLightSourceEvent(const LightSourceEvent& event);

		/** Handles the given RenderableShaderEvent by updating the
		 * Renderables uniforms
		 *
		 * @param	event the RenderableShaderEvent to handle */
		void onRenderableShaderEvent(const RenderableShaderEvent& event);

		/** Handles the given ShaderEvent by updating the Renderables uniforms
		 *
		 * @param	event the ShaderEvent to handle */
		void onShaderEvent(const ShaderEvent& event);

		/** Removes the Mesh, Bindables and Techniques from the given Entity
		 * LightComponent
		 *
		 * @param	entity the Entity that holds the LightComponent to update
		 * @param	light a pointer to the LightComponent to update */
		void clearRMesh(Entity entity, LightComponent* light);

		/** Adds the Mesh, Bindables and Techniques to the given Entity
		 * LightComponent based on its current source
		 *
		 * @param	entity the Entity that holds the LightComponent to update
		 * @param	light a pointer to the LightComponent to update */
		void setRMesh(Entity entity, LightComponent* light);

		/** Calculates the cascade shadow mapping Z-Far split depths
		 *
		 * @param	zNear the camera Z-near used for rendering the Shadows
		 * @param	zFar the camera Z-far used for rendering the Shadows
		 * @param	numCascades the number of cascades used for CSM */
		utils::FixedVector<float, RenderableLight::kMaxShadowMaps + 1>
			calculateCascadesZFar(
				float zNear, float zFar, std::size_t numCascades
			);
	};

}

#endif		// LIGHT_SYSTEM_H
