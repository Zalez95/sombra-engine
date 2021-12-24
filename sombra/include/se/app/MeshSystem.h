#ifndef MESH_SYSTEM_H
#define MESH_SYSTEM_H

#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include "../graphics/core/UniformVariable.h"
#include "events/RMeshEvent.h"
#include "events/ShaderEvent.h"
#include "events/RenderableShaderEvent.h"
#include "events/EventManager.h"
#include "ECS.h"
#include "MeshComponent.h"

namespace se::app {

	class Application;


	/**
	 * Class MeshSystem, it's a System used for updating the Entities'
	 * RenderableMesh data
	 */
	class MeshSystem : public ISystem, IEventListener
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
			std::shared_ptr<graphics::UniformVariableValueVector<glm::mat3x4>>
				jointMatrices;
		};

		using EntityUniformsVector = std::vector<EntityUniforms>;

	private:	// Attributes
		/** The Application that holds the GraphicsEngine used for rendering
		 * the RenderableMeshes */
		Application& mApplication;

		/** All the uniforms to update of each Entity */
		std::unordered_map<
			Entity,
			std::array<EntityUniformsVector, MeshComponent::kMaxMeshes>
		> mEntityUniforms;

	public:		// Functions
		/** Creates a new MeshSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System */
		MeshSystem(Application& application);

		/** Class destructor */
		~MeshSystem();

		/** @copydoc IEventListener::notify(const IEvent&) */
		virtual bool notify(const IEvent& event) override;

		/** @copydoc ISystem::onNewComponent(Entity, const EntityDatabase::ComponentMask&) */
		virtual void onNewComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask
		) override
		{ tryCallC(&MeshSystem::onNewMesh, entity, mask); };

		/** @copydoc ISystem::onRemoveComponent(Entity, const EntityDatabase::ComponentMask&) */
		virtual void onRemoveComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask
		) override
		{ tryCallC(&MeshSystem::onRemoveMesh, entity, mask); };

		/** Updates the RenderableMeshes with the Entities */
		virtual void update() override;
	private:
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

		/** Handles the given RMeshEvent by updating the RenderableMeshes
		 * uniforms
		 *
		 * @param	event the RMeshEvent to handle */
		void onRMeshEvent(const RMeshEvent& event);

		/** Handles the given RenderableShaderEvent by updating the
		 * RenderableMeshes uniforms
		 *
		 * @param	event the RenderableShaderEvent to handle */
		void onRenderableShaderEvent(const RenderableShaderEvent& event);

		/** Handles the given ShaderEvent by updating the RenderableMeshes
		 * uniforms
		 *
		 * @param	event the ShaderEvent to handle */
		void onShaderEvent(const ShaderEvent& event);

		/** Adds the RenderableShaderStep uniforms to the Entity mesh
		 *
		 * @param	entity the Entity that owns the RenderableMesh
		 * @param	rIndex the index of the RenderableMesh updated
		 * @param	step a pointer to the new RenderableShaderStep to add */
		void addStep(
			Entity entity, std::size_t rIndex,
			const RenderableShaderStepSPtr& step
		);

		/** Removes the RenderableShaderStep uniforms from the Entity mesh
		 *
		 * @param	entity the Entity that owns the RenderableMesh
		 * @param	rIndex the index of the RenderableMesh updated
		 * @param	step a pointer to the RenderableShaderStep to remove */
		void removeStep(
			Entity entity, std::size_t rIndex,
			const RenderableShaderStepSPtr& step
		);
	};

}

#endif		// MESH_SYSTEM_H
