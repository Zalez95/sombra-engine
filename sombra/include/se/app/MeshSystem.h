#ifndef MESH_SYSTEM_H
#define MESH_SYSTEM_H

#include <deque>
#include <unordered_map>
#include <memory>
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
	class MeshSystem : public ISystem, public IEventListener
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

		struct StepOperation
		{
			enum class Operation { Add, Remove };
			Operation operation;
			Entity entity;
			std::size_t rIndex;
			RenderableShaderStepSPtr step;
		};

		using EntityUniformsVector = std::vector<EntityUniforms>;

	private:	// Attributes
		/** The Application that holds the GraphicsEngine used for rendering
		 * the RenderableMeshes */
		Application& mApplication;

		/** Maps each Entity with its uniforms to update */
		std::unordered_map<
			Entity,
			std::array<EntityUniformsVector, MeshComponent::kMaxMeshes>
		> mEntityUniforms;

		/** Holds all the operations with the steps that must be performed by
		 * thread 0 */
		std::deque<StepOperation> mStepOperationsQueue;

		/** The mutex that protects @see mEntityUniforms and
		 * @see mStepOperationsQueue */
		std::mutex mMutex;

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

		/** @copydoc ISystem::onNewComponent(Entity, const EntityDatabase::ComponentMask&, EntityDatabase::Query&) */
		virtual void onNewComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask,
			EntityDatabase::Query& query
		) override
		{ tryCallC(&MeshSystem::onNewMesh, entity, mask, query); };

		/** @copydoc ISystem::onRemoveComponent(Entity, const EntityDatabase::ComponentMask&, EntityDatabase::Query&) */
		virtual void onRemoveComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask,
			EntityDatabase::Query& query
		) override
		{ tryCallC(&MeshSystem::onRemoveMesh, entity, mask, query); };

		/** Updates the RenderableMeshes with the Entities */
		virtual void update() override;
	private:
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
		 * @param	query the Query object used for interacting with the Entity
		 *			and its other Components
		 * @param	step a pointer to the new RenderableShaderStep to add */
		void addStep(
			Entity entity, std::size_t rIndex, EntityDatabase::Query& query,
			const RenderableShaderStepSPtr& step
		);

		/** Removes the RenderableShaderStep uniforms from the Entity mesh
		 *
		 * @param	entity the Entity that owns the RenderableMesh
		 * @param	rIndex the index of the RenderableMesh updated
		 * @param	query the Query object used for interacting with the Entity
		 *			and its other Components
		 * @param	step a pointer to the RenderableShaderStep to remove */
		void removeStep(
			Entity entity, std::size_t rIndex, EntityDatabase::Query& query,
			const RenderableShaderStepSPtr& step
		);
	};

}

#endif		// MESH_SYSTEM_H
