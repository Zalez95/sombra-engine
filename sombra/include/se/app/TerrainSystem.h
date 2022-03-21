#ifndef TERRAIN_SYSTEM_H
#define TERRAIN_SYSTEM_H

#include <queue>
#include <unordered_map>
#include <memory>
#include <future>
#include <glm/glm.hpp>
#include "events/ContainerEvent.h"
#include "events/ShaderEvent.h"
#include "events/RenderableShaderEvent.h"
#include "events/EventManager.h"
#include "ECS.h"

namespace se::app {

	class Application;
	struct TransformsComponent;
	class TerrainComponent;


	/**
	 * Class TerrainSystem, it's a System used for updating the Entities'
	 * RenderableTerrain data
	 */
	class TerrainSystem : public ISystem, public IEventListener
	{
	private:	// Nested types
		using RenderableShaderStepSPtr = std::shared_ptr<RenderableShaderStep>;
		using RenderableShaderSPtr = std::shared_ptr<RenderableShader>;

		struct EntityUniforms
		{
			std::size_t shaderCount = 0;
			RenderableShaderStepSPtr step;
			UniformVVRef<glm::mat4> modelMatrix;
		};

		using EntityUniformsVector = std::vector<EntityUniforms>;

		struct NewUniform
		{
			Entity entity;
			RenderableShaderStepSPtr step;
			UniformVVRef<glm::mat4> modelMatrix;
			std::future<bool> modelMatrixFound;
		};

	private:	// Attributes
		/** The Application that holds the GraphicsEngine used for rendering
		 * the RenderableTerrains */
		Application& mApplication;

		/** All the uniforms to update of each Entity */
		std::unordered_map<Entity, EntityUniformsVector> mEntityUniforms;

		/** The camera Entity used for rendering */
		Entity mCameraEntity;

		/** The last position of the camera Entity */
		glm::vec3 mLastCameraPosition;

		/** The mutex that protects @see mEntityUniforms, @see mCameraEntity and
		 * @see mCameraUpdated */
		std::mutex mMutex;

		/** The new uniforms to add to the terrain entities, it's needed because
		 * we can't use the EntityDatabase inside the Context functions */
		std::queue<NewUniform> mNewUniforms;

		/** The mutex that protects @see mNewUniforms */
		std::mutex mUniformsMutex;

	public:		// Functions
		/** Creates a new TerrainSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System */
		TerrainSystem(Application& application);

		/** Class destructor */
		~TerrainSystem();

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

		/** Updates the RenderableTerrains with the Entities */
		virtual void update() override;
	private:
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

		/** Function called when a TerrainComponent is going to be removed from
		 * an Entity
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

		/** Function called when a TransformsComponent is added to an Entity
		 *
		 * @param	entity the Entity that holds the TransformsComponent
		 * @param	transforms a pointer to the new TransformsComponent
		 * @param	query the Query object used for interacting with the Entity
		 *			and its other Components */
		void onNewTransforms(
			Entity entity, TransformsComponent* transforms,
			EntityDatabase::Query& query
		);

		/** Handles the given ContainerEvent by updating the Camera Entity with
		 * which the Scene will be rendered
		 *
		 * @param	event the ContainerEvent to handle */
		void onCameraEvent(const ContainerEvent<Topic::Camera, Entity>& event);

		/** Handles the given RenderableShaderEvent by updating the
		 * RenderableTerrain uniforms
		 *
		 * @param	event the RenderableShaderEvent to handle */
		void onRenderableShaderEvent(const RenderableShaderEvent& event);

		/** Handles the given ShaderEvent by updating the RenderableTerrain
		 * uniforms
		 *
		 * @param	event the ShaderEvent to handle */
		void onShaderEvent(const ShaderEvent& event);

		/** Adds the RenderableShaderStep uniforms to the Entity terrain
		 *
		 * @param	entity the Entity that owns the TerrainComponent
		 * @param	query the Query object used for interacting with the Entity
		 *			and its other Components
		 * @param	step a pointer to the new RenderableShaderStep to add */
		void addStep(
			Entity entity, EntityDatabase::Query& query,
			const RenderableShaderStepSPtr& step
		);

		/** Removes the RenderableShaderStep uniforms from the Entity terrain
		 *
		 * @param	entity the Entity that owns the TerrainComponent
		 * @param	query the Query object used for interacting with the Entity
		 *			and its other Components
		 * @param	step a pointer to the RenderableShaderStep to remove */
		void removeStep(
			Entity entity, EntityDatabase::Query& query,
			const RenderableShaderStepSPtr& step
		);
	};

}

#endif		// TERRAIN_SYSTEM_H
