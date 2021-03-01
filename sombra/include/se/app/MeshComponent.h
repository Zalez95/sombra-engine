#ifndef MESH_COMPONENT_H
#define MESH_COMPONENT_H

#include <array>
#include "../graphics/3D/RenderableMesh.h"
#include "RenderableShader.h"
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
		using RenderableShaderSPtr = std::shared_ptr<RenderableShader>;

		struct RMesh
		{
			bool active = false;
			bool hasSkinning = false;
			graphics::RenderableMesh renderable;
			std::vector<RenderableShaderSPtr> shaders;
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

		/** Returns if the selected RenderableMesh has skinning or not
		 *
		 * @param	rIndex the index of the RenderableMesh
		 * @return	true if the RenderableMesh has skinning, false otherwise */
		bool hasSkinning(std::size_t rIndex) const
		{ return mRMeshes[rIndex].hasSkinning; };

		/** Adds a new RenderableMesh to the RenderableComponent
		 *
		 * @param	hasSkinning if the new mesh has skinning or not
		 * @param	mesh a pointer to the Mesh of the RenderableMesh
		 * @param	primitiveMesh the type of primitive used for rendering
		 * @return	the index used for accesing the new RenderableMesh */
		std::size_t add(
			bool hasSkinning,
			std::shared_ptr<graphics::Mesh> mesh = nullptr,
			graphics::PrimitiveType primitiveType =
				graphics::PrimitiveType::Triangle
		);

		/** Iterates through all the RenderableMesh indices calling the given
		 * callback function
		 *
		 * @param	callback the function to call for each RenderableMesh
		 *			index */
		template <typename F>
		void processRenderableIndices(F callback) const;

		/** Removes the selected RenderableMesh
		 *
		 * @param	rIndex the index of the RenderableMesh to remove */
		void remove(std::size_t rIndex);

		/** Adds the given RenderableShader to the selected RenderableMesh
		 *
		 * @param	rIndex the index of the RenderableMesh to update
		 * @param	shader a pointer to the shader to add */
		void addRenderableShader(
			std::size_t rIndex, const RenderableShaderSPtr& shader
		);

		/** Iterates through all the RenderableShaders of the given
		 * RenderableMesh calling the given callback function
		 *
		 * @param	rIndex the index of the RenderableMesh to process
		 * @param	callback the function to call for each RenderableShader */
		template <typename F>
		void processRenderableShaders(std::size_t rIndex, F callback) const;

		/** Removes the given RenderableShader from the selected RenderableMesh
		 *
		 * @param	rIndex the index of the RenderableMesh to update
		 * @param	shader a pointer to the shader to remove */
		void removeRenderableShader(
			std::size_t rIndex, const RenderableShaderSPtr& shader
		);
	};


	template <typename F>
	void MeshComponent::processRenderableIndices(F callback) const
	{
		for (std::size_t i = 0; i < mRMeshes.size(); ++i) {
			if (mRMeshes[i].active) {
				callback(i);
			}
		}
	}


	template <typename F>
	void MeshComponent::processRenderableShaders(
		std::size_t rIndex, F callback
	) const
	{
		for (auto& shader : mRMeshes[rIndex].shaders) {
			callback(shader);
		}
	}

}

#endif		// MESH_COMPONENT_H
