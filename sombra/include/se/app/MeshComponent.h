#ifndef MESH_COMPONENT_H
#define MESH_COMPONENT_H

#include <array>
#include "../graphics/3D/RenderableMesh.h"
#include "graphics/RenderableShader.h"
#include "Entity.h"

namespace se::app {

	class EventManager;


	/**
	 * Class MeshComponent, it's a Component that holds all the graphics
	 * RenderableMeshes of an Entity.
	 */
	class MeshComponent
	{
	private:	// Nested types
		using MeshResource = Repository::ResourceRef<MeshRef>;
		using RenderableShaderResource =
			Repository::ResourceRef<RenderableShader>;

		struct RMesh
		{
			bool active = false;
			bool hasSkinning = false;
			MeshResource mesh;
			std::vector<RenderableShaderResource> shaders;
			graphics::RenderableMesh renderable;
		};

	public:		// Attributes
		/** The maximum number of RenderableMeshes that the MeshComponent can
		 * hold */
		static constexpr std::size_t kMaxMeshes = 128;
	private:
		/** The EventManager used for notifying the MeshComponent changes */
		EventManager* mEventManager = nullptr;

		/** The Entity that owns the MeshComponent */
		Entity mEntity = kNullEntity;

		/** All the RenderableMeshes added to the MeshComponent */
		std::array<RMesh, kMaxMeshes> mRMeshes;

	public:		// Functions
		/** Creates a new MeshComponent */
		MeshComponent() = default;
		MeshComponent(const MeshComponent& other);
		MeshComponent(MeshComponent&& other) = default;

		/** Class destructor */
		~MeshComponent() = default;

		/** Assignment operator */
		MeshComponent& operator=(const MeshComponent& other);
		MeshComponent& operator=(MeshComponent&& other) = default;

		/** Sets the MeshComponent attributes
		 *
		 * @param	eventManager the new EventManager of the MeshComponent
		 * @param	entity the new Entity of the MeshComponent */
		void setup(EventManager* eventManager, Entity entity);

		/** @return	true if no more RenderableMeshes can be added, false
		 *			otherwise */
		bool full() const;

		/** @return	true if there aren't any RenderableMeshes added, false
		 *			otherwise */
		bool empty() const;

		/** @return	true if there is at least one RenderableMesh added, false
		 *			otherwise */
		bool any() const;

		/** Returns if the selected RenderableMesh is active or not
		 *
		 * @param	rIndex the index of the RenderableMesh
		 * @return	true if it's active, false otherwise */
		bool isActive(std::size_t rIndex) const
		{ return mRMeshes[rIndex].active; };

		/** Returns if the selected RenderableMesh has skinning or not
		 *
		 * @param	rIndex the index of the RenderableMesh
		 * @return	true if the RenderableMesh has skinning, false otherwise */
		bool hasSkinning(std::size_t rIndex) const
		{ return mRMeshes[rIndex].hasSkinning; };

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

		/** Returns the selected Mesh
		 *
		 * @param	rIndex the index of the Mesh
		 * @return	the Mesh */
		const MeshResource& getMesh(std::size_t rIndex) const
		{ return mRMeshes[rIndex].mesh; };

		/** Sets the Mesh of the given RenderableMesh
		 *
		 * @param	rIndex the index of the RenderableMesh
		 * @param	mesh a pointer to the new Mesh of the RenderableMesh */
		void setMesh(std::size_t rIndex, const MeshResource& mesh);

		/** Adds a new RenderableMesh to the RenderableComponent
		 *
		 * @param	hasSkinning if the new mesh has skinning or not
		 * @param	mesh a pointer to the Mesh of the RenderableMesh
		 * @param	primitiveType the type of primitive used for rendering
		 * @return	the index used for accesing the new RenderableMesh */
		std::size_t add(
			bool hasSkinning = false,
			const MeshResource& mesh = {},
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

		/** Removes all the Meshes from the MeshComponent */
		void clear();

		/** Adds the given RenderableShader to the selected RenderableMesh
		 *
		 * @param	rIndex the index of the RenderableMesh to update
		 * @param	shader a pointer to the shader to add */
		void addRenderableShader(
			std::size_t rIndex, const RenderableShaderResource& shader
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
			std::size_t rIndex, const RenderableShaderResource& shader
		);
	};

}

#endif		// MESH_COMPONENT_H
