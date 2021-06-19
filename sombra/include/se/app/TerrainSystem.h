#ifndef TERRAIN_SYSTEM_H
#define TERRAIN_SYSTEM_H

#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include "../graphics/core/UniformVariable.h"
#include "ECS.h"
#include "events/ContainerEvent.h"
#include "events/ShaderEvent.h"
#include "events/RenderableShaderEvent.h"

namespace se::app {

	class Application;
	class TerrainComponent;


	/**
	 * Class TerrainSystem, it's a System used for updating the Entities'
	 * RenderableTerrain data
	 */
	class TerrainSystem : public ISystem
	{
	private:	// Nested types
		using RenderableShaderStepSPtr = std::shared_ptr<RenderableShaderStep>;
		using RenderableShaderSPtr = std::shared_ptr<RenderableShader>;

		struct EntityUniforms
		{
			std::size_t shaderCount = 0;
			RenderableShaderStepSPtr step;
			std::shared_ptr<graphics::UniformVariableValue<glm::mat4>>
				modelMatrix;
		};

		using EntityUniformsVector = std::vector<EntityUniforms>;

	private:	// Attributes
		/** The Application that holds the GraphicsEngine used for rendering
		 * the RenderableTerrains */
		Application& mApplication;

		/** All the uniforms to update of each Entity */
		std::unordered_map<Entity, EntityUniformsVector> mEntityUniforms;

		/** The camera Entity used for rendering */
		Entity mCameraEntity;

		/** If the camera was updated or not */
		bool mCameraUpdated;

	public:		// Functions
		/** Creates a new TerrainSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System */
		TerrainSystem(Application& application);

		/** Class destructor */
		~TerrainSystem();

		/** @copydoc ISystem::notify(const IEvent&) */
		virtual bool notify(const IEvent& event) override;

		/** @copydoc ISystem::onNewComponent(Entity, const EntityDatabase::ComponentMask&) */
		virtual void onNewComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask
		) override
		{ tryCallC(&TerrainSystem::onNewTerrain, entity, mask); };

		/** @copydoc ISystem::onRemoveComponent(Entity, const EntityDatabase::ComponentMask&) */
		virtual void onRemoveComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask
		) override
		{ tryCallC(&TerrainSystem::onRemoveTerrain, entity, mask); };

		/** Updates the RenderableTerrains with the Entities */
		virtual void update() override;
	private:
		/** Function called when a TerrainComponent is added to an Entity
		 *
		 * @param	entity the Entity that holds the TerrainComponent
		 * @param	terrain a pointer to the new TerrainComponent */
		void onNewTerrain(Entity entity, TerrainComponent* terrain);

		/** Function called when a TerrainComponent is going to be removed from
		 * an Entity
		 *
		 * @param	entity the Entity that holds the TerrainComponent
		 * @param	terrain a pointer to the TerrainComponent that is going to
		 *			be removed */
		void onRemoveTerrain(Entity entity, TerrainComponent* terrain);

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
		 * @param	step a pointer to the new RenderableShaderStep to add */
		void addStep(Entity entity, const RenderableShaderStepSPtr& step);

		/** Removes the RenderableShaderStep uniforms from the Entity terrain
		 *
		 * @param	entity the Entity that owns the TerrainComponent
		 * @param	step a pointer to the RenderableShaderStep to remove */
		void removeStep(Entity entity, const RenderableShaderStepSPtr& step);
	};

}

#endif		// TERRAIN_SYSTEM_H
