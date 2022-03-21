#ifndef MESH_SYSTEM_H
#define MESH_SYSTEM_H

#include <queue>
#include <unordered_map>
#include <memory>
#include <future>
#include <glm/glm.hpp>
#include "events/RMeshEvent.h"
#include "events/ShaderEvent.h"
#include "events/RenderableShaderEvent.h"
#include "events/EventManager.h"
#include "ECS.h"
#include "MeshComponent.h"

namespace se::app {

	class Application;
	struct TransformsComponent;


	/**
	 * Class MeshSystem, it's a System used for updating the Entities'
	 * RenderableMesh data
	 */
	class MeshSystem : public ISystem, public IEventListener
	{
	private:	// Nested types
		using RenderableShaderStepSPtr = std::shared_ptr<RenderableShaderStep>;
		using RenderableShaderSPtr = std::shared_ptr<RenderableShader>;

		/** Holds the uniforms added to an step */
		struct EntityUniforms
		{
			std::size_t shaderCount = 0;
			RenderableShaderStepSPtr step;
			UniformVVRef<glm::mat4> modelMatrix;
			UniformVVVRef<glm::mat3x4> jointMatrices;
		};

		/** Holds the data of the new uniforms to process */
		struct NewUniform
		{
			Entity entity;
			std::size_t rIndex;
			RenderableShaderStepSPtr step;
			graphics::Context::BindableRef uniform;
			std::future<bool> uniformFound;
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

		/** The mutex that protects @see mEntityUniforms */
		std::mutex mMutex;

		/** The new uniforms to add to the mesh entities, it's needed because
		 * we can't use the EntityDatabase inside the Context functions */
		std::queue<NewUniform> mNewUniforms;

		/** The mutex that protects @see mNewUniforms */
		std::mutex mUniformsMutex;

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
		) override;

		/** @copydoc ISystem::onRemoveComponent(Entity, const EntityDatabase::ComponentMask&, EntityDatabase::Query&) */
		virtual void onRemoveComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask,
			EntityDatabase::Query& query
		) override;

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
