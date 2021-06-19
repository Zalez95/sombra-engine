#ifndef MESH_COMPONENT_H
#define MESH_COMPONENT_H

#include <array>
#include "../graphics/3D/RenderableMesh.h"
#include "graphics/RenderableShader.h"
#include "events/EventManager.h"
#include "events/RMeshEvent.h"
#include "Entity.h"

namespace se::app {

	/**
	 * Class MeshComponent, it's a Component that holds all the graphics
	 * RenderableMeshes of an Entity.
	 */
	class MeshComponent
	{
	private:	// Nested types
		using MeshRef = Repository::ResourceRef<graphics::Mesh>;
		using RenderableShaderRef = Repository::ResourceRef<RenderableShader>;

		struct RMesh
		{
			bool active = false;
			bool hasSkinning = false;
			MeshRef mesh;
			std::vector<RenderableShaderRef> shaders;
			graphics::RenderableMesh renderable;
		};

	public:		// Attributes
		/** The maximum number of RenderableMeshes that the MeshComponent can
		 * hold */
		static constexpr std::size_t kMaxMeshes = 128;
	private:
		/** The EventManager used for notifying the MeshComponent changes */
		EventManager& mEventManager;

		/** The Entity that owns the MeshComponent */
		Entity mEntity;

		/** All the RenderableMeshes added to the MeshComponent */
		std::array<RMesh, kMaxMeshes> mRMeshes;

	public:		// Functions
		/** Creates a new MeshComponent
		 *
		 * @param	eventManager the EventManager used for notifying the
		 *			MeshComponent changes
		 * @param	entity the Entity that owns the new MeshComponent */
		MeshComponent(EventManager& eventManager, Entity entity) :
			mEventManager(eventManager), mEntity(entity) {};

		/** @return	true if no more RenderableMeshes can be added, false
		 *			otherwise */
		bool full() const;

		/** Returns the selected RenderableMesh
		 *
		 * @param	rIndex the index of the RenderableMesh
		 * @return	the RenderableMesh */
		graphics::RenderableMesh& get(std::size_t rIndex)
		{ return mRMeshes[rIndex].renderable; };

		/** Returns the selected RenderableMesh
		 *
		 * @param	rIndex the index of the RenderableMesh
		 * @return	the RenderableMesh */
		const graphics::RenderableMesh& get(std::size_t rIndex) const
		{ return mRMeshes[rIndex].renderable; };

		/** Returns if the selected RenderableMesh has skinning or not
		 *
		 * @param	rIndex the index of the RenderableMesh
		 * @return	true if the RenderableMesh has skinning, false otherwise */
		bool hasSkinning(std::size_t rIndex) const
		{ return mRMeshes[rIndex].hasSkinning; };

		/** Returns the selected Mesh
		 *
		 * @param	rIndex the index of the Mesh
		 * @return	the Mesh */
		const MeshRef& getMesh(std::size_t rIndex) const
		{ return mRMeshes[rIndex].mesh; };

		/** Sets the Mesh of the given RenderableMesh
		 *
		 * @param	rIndex the index of the RenderableMesh
		 * @param	mesh a pointer to the new Mesh of the RenderableMesh */
		void setMesh(std::size_t rIndex, MeshRef mesh);

		/** Adds a new RenderableMesh to the RenderableComponent
		 *
		 * @param	hasSkinning if the new mesh has skinning or not
		 * @param	mesh a pointer to the Mesh of the RenderableMesh
		 * @param	primitiveType the type of primitive used for rendering
		 * @return	the index used for accesing the new RenderableMesh */
		std::size_t add(
			bool hasSkinning = false,
			MeshRef mesh = MeshRef(),
			graphics::PrimitiveType primitiveType =
				graphics::PrimitiveType::Triangle
		);

		/** Iterates through all the RenderableMesh indices calling the given
		 * callback function
		 *
		 * @param	callback the function to call for each RenderableMesh
		 *			index */
		template <typename F>
		void processRenderableIndices(F&& callback) const
		{
			for (std::size_t i = 0; i < mRMeshes.size(); ++i) {
				if (mRMeshes[i].active) {
					callback(i);
				}
			}
		}

		/** Removes the selected RenderableMesh
		 *
		 * @param	rIndex the index of the RenderableMesh to remove */
		void remove(std::size_t rIndex);

		/** Adds the given RenderableShader to the selected RenderableMesh
		 *
		 * @param	rIndex the index of the RenderableMesh to update
		 * @param	shader a pointer to the shader to add */
		void addRenderableShader(
			std::size_t rIndex, const RenderableShaderRef& shader
		);

		/** Iterates through all the RenderableShaders of the given
		 * RenderableMesh calling the given callback function
		 *
		 * @param	rIndex the index of the RenderableMesh to process
		 * @param	callback the function to call for each RenderableShader */
		template <typename F>
		void processRenderableShaders(std::size_t rIndex, F&& callback) const
		{
			for (auto& shader : mRMeshes[rIndex].shaders) {
				callback(shader);
			}
		}

		/** Removes the given RenderableShader from the selected RenderableMesh
		 *
		 * @param	rIndex the index of the RenderableMesh to update
		 * @param	shader a pointer to the shader to remove */
		void removeRenderableShader(
			std::size_t rIndex, const RenderableShaderRef& shader
		);
	};

}

#endif		// MESH_COMPONENT_H
