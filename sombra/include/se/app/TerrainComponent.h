#ifndef TERRAIN_COMPONENT_H
#define TERRAIN_COMPONENT_H

#include "../graphics/3D/RenderableTerrain.h"
#include "graphics/RenderableShader.h"
#include "events/EventManager.h"
#include "Entity.h"

namespace se::app {

	/**
	 * Class TerrainComponent, it's a Component that holds the graphics
	 * RenderableTerrain of an Entity.
	 */
	class TerrainComponent
	{
	private:	// Nested types
		using RenderableShaderResource =
			Repository::ResourceRef<RenderableShader>;

	private:	// Attributes
		/** The EventManager used for notifying the TerrainComponent changes */
		EventManager* mEventManager;

		/** The Entity that owns the TerrainComponent */
		Entity mEntity;

		/** The RenderableTerrain of the TerrainComponent */
		graphics::RenderableTerrain mRenderableTerrain;

		/** The shaders added to the TerrainComponent */
		std::vector<RenderableShaderResource> mShaders;

	public:		// Functions
		/** Creates a new TerrainComponent
		 *
		 * @param	size the size of the terrain in the XZ plane
		 * @param	maxHeight the maximum height of the vertices of the terrain
		 * @param	lodDistances the minimum distance to the camera at each
		 *			level of detail */
		TerrainComponent(
			float size = 0.0f, float maxHeight = 0.0f,
			const std::vector<float>& lodDistances = {}
		) : mEventManager(nullptr), mEntity(kNullEntity),
			mRenderableTerrain(size, maxHeight, lodDistances) {};
		TerrainComponent(const TerrainComponent& other);
		TerrainComponent(TerrainComponent&& other) = default;

		/** Class destructor */
		~TerrainComponent() = default;

		/** Assignment operator */
		TerrainComponent& operator=(const TerrainComponent& other);
		TerrainComponent& operator=(TerrainComponent&& other) = default;

		/** Sets the TerrainComponent attributes
		 *
		 * @param	eventManager the new EventManager of the TerrainComponent
		 * @param	entity the new Entity of the TerrainComponent */
		void setup(EventManager* eventManager, Entity entity)
		{
			mEventManager = eventManager;
			mEntity = entity;
		};

		/** @return	the RenderableTerrain of the TerrainComponent */
		graphics::RenderableTerrain& get() { return mRenderableTerrain; };

		/** @return	the RenderableTerrain of the TerrainComponent */
		const graphics::RenderableTerrain& get() const
		{ return mRenderableTerrain; };

		/** Adds the given RenderableShader to the TerrainComponent
		 *
		 * @param	shader a pointer to the shader to add */
		void addRenderableShader(const RenderableShaderResource& shader);

		/** Iterates through all the RenderableShaders of the TerrainComponents
		 * calling the given callback function
		 *
		 * @param	callback the function to call for each RenderableShader */
		template <typename F>
		void processRenderableShaders(F&& callback) const;

		/** Removes the given RenderableShader from the TerrainComponent
		 *
		 * @param	shader a pointer to the shader to remove */
		void removeRenderableShader(const RenderableShaderResource& shader);
	};


	template <typename F>
	void TerrainComponent::processRenderableShaders(F&& callback) const
	{
		for (auto& shader : mShaders) {
			callback(shader);
		}
	}

}

#endif		// TERRAIN_COMPONENT_H
