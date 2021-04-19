#ifndef MESH_SYSTEM_H
#define MESH_SYSTEM_H

#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include "../graphics/Pass.h"
#include "../graphics/core/UniformVariable.h"
#include "ISystem.h"
#include "MeshComponent.h"
#include "events/RMeshEvent.h"
#include "events/ShaderEvent.h"
#include "events/RenderableShaderEvent.h"

namespace se::app {

	class Application;


	/**
	 * Class MeshSystem, it's a System used for updating the Entities'
	 * RenderableMesh data
	 */
	class MeshSystem : public ISystem
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

		/** Notifies the MeshSystem of the given event
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

		/** Updates the RenderableMeshes with the Entities */
		virtual void update() override;
	private:
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

		/** Adds the pass uniforms to the Entity mesh
		 *
		 * @param	entity the Entity that owns the RenderableMesh
		 * @param	rIndex the index of the RenderableMesh updated
		 * @param	pass a pointer to the new Pass to add */
		void addPass(Entity entity, std::size_t rIndex, const PassSPtr& pass);

		/** Removes the pass uniforms from the Entity mesh
		 *
		 * @param	entity the Entity that owns the RenderableMesh
		 * @param	rIndex the index of the RenderableMesh updated
		 * @param	pass a pointer to the Pass to remove */
		void removePass(
			Entity entity, std::size_t rIndex, const PassSPtr& pass
		);
	};

}

#endif		// MESH_SYSTEM_H
