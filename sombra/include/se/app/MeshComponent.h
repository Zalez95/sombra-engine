#ifndef MESH_COMPONENT_H
#define MESH_COMPONENT_H

#include "../utils/FixedVector.h"
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
			bool hasSkinning;
			graphics::RenderableMesh renderable;
			std::vector<RenderableShaderSPtr> shaders;

			template <typename... Args>
			RMesh(bool hasSkinning, Args&&... args) :
				hasSkinning(hasSkinning),
				renderable(std::forward<Args>(args)...) {}
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
		utils::FixedVector<RMesh, kMaxMeshes> mRMeshes;

	public:		// Functions
		/** Creates a new MeshComponent
		 *
		 * @param	eventManager the EventManager used for notifying the
		 *			MeshComponent changes
		 * @param	entity the Entity that owns the new MeshComponent */
		MeshComponent(EventManager& eventManager, Entity entity) :
			mEventManager(eventManager), mEntity(entity) {};

		/** @return	the number of RenderableMeshes added to the
		 *			RenderableComponent */
		std::size_t size() const { return mRMeshes.size(); };

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
		 * @param	args the arguments needed for calling the constructor of
		 *			the new RenderableMesh
		 * @return	the index used for accesing the new RenderableMeshe */
		template <typename... Args>
		std::size_t add(bool hasSkinning, Args&&... args);

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


	template <typename... Args>
	std::size_t MeshComponent::add(bool hasSkinning, Args&&... args)
	{
		mRMeshes.emplace_back(hasSkinning, std::forward<Args>(args)...);
		std::size_t ret = mRMeshes.size() - 1;
		mEventManager.publish(
			new RMeshEvent(RMeshEvent::Operation::Add, mEntity, ret)
		);
		return ret;
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
