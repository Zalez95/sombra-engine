#ifndef TERRAIN_SYSTEM_H
#define TERRAIN_SYSTEM_H

#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include "../graphics/Pass.h"
#include "../graphics/core/UniformVariable.h"
#include "ISystem.h"
#include "events/ContainerEvent.h"
#include "events/ShaderEvent.h"
#include "events/RenderableShaderEvent.h"

namespace se::app {

	class Application;


	/**
	 * Class TerrainSystem, it's a System used for updating the Entities'
	 * RenderableTerrain data
	 */
	class TerrainSystem : public ISystem
	{
	private:	// Nested types
		using PassSPtr = std::shared_ptr<graphics::Pass>;
		using RenderableShaderSPtr = std::shared_ptr<RenderableShader>;

		struct EntityUniforms
		{
			std::size_t shaderCount = 0;
			PassSPtr pass;
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

		/** Notifies the TerrainSystem of the given event
		 *
		 * @param	event the IEvent to notify */
		virtual void notify(const IEvent& event) override;

		/** Function that the EntityDatabase will call when an Entity is
		 * added
		 *
		 * @param	entity the new Entity */
		virtual void onNewEntity(Entity entity);

		/** Function that the EntityDatabase will call when an Entity is
		 * removed
		 *
		 * @param	entity the Entity to remove */
		virtual void onRemoveEntity(Entity entity);

		/** Updates the RenderableTerrains with the Entities */
		virtual void update() override;
	private:
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

		/** Adds the pass uniforms to the Entity terrain
		 *
		 * @param	entity the Entity that owns the TerrainComponent
		 * @param	pass a pointer to the new Pass to add */
		void addPass(Entity entity, const PassSPtr& pass);

		/** Removes the pass uniforms from the Entity terrain
		 *
		 * @param	entity the Entity that owns the TerrainComponent
		 * @param	pass a pointer to the Pass to remove */
		void removePass(Entity entity, const PassSPtr& pass);
	};

}

#endif		// TERRAIN_SYSTEM_H
