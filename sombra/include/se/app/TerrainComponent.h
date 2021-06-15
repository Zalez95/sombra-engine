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
		using RenderableShaderRef = Repository::ResourceRef<RenderableShader>;

	private:	// Attributes
		/** The EventManager used for notifying the TerrainComponent changes */
		EventManager& mEventManager;

		/** The Entity that owns the TerrainComponent */
		Entity mEntity;

		/** The RenderableTerrain of the TerrainComponent */
		graphics::RenderableTerrain mRenderableTerrain;

		/** The shaders added to the TerrainComponent */
		std::vector<RenderableShaderRef> mShaders;

	public:		// Functions
		/** Creates a new TerrainComponent
		 *
		 * @param	eventManager the EventManager used for notifying the
		 *			TerrainComponent changes
		 * @param	entity the Entity that owns the new TerrainComponent
		 * @param	size the size of the terrain in the XZ plane
		 * @param	maxHeight the maximum height of the vertices of the terrain
		 * @param	lodDistances the minimum distance to the camera at each
		 *			level of detail */
		TerrainComponent(
			EventManager& eventManager, Entity entity,
			float size = 0.0f, float maxHeight = 0.0f,
			const std::vector<float>& lodDistances = {}
		) : mEventManager(eventManager), mEntity(entity),
			mRenderableTerrain(size, maxHeight, lodDistances) {};

		/** @return	the RenderableTerrain of the TerrainComponent */
		graphics::RenderableTerrain& get() { return mRenderableTerrain; };

		/** @return	the RenderableTerrain of the TerrainComponent */
		const graphics::RenderableTerrain& get() const
		{ return mRenderableTerrain; };

		/** Adds the given RenderableShader to the TerrainComponent
		 *
		 * @param	shader a pointer to the shader to add */
		void addRenderableShader(const RenderableShaderRef& shader);

		/** Iterates through all the RenderableShaders of the TerrainComponents
		 * calling the given callback function
		 *
		 * @param	callback the function to call for each RenderableShader */
		template <typename F>
		void processRenderableShaders(F callback) const;

		/** Removes the given RenderableShader from the TerrainComponent
		 *
		 * @param	shader a pointer to the shader to remove */
		void removeRenderableShader(const RenderableShaderRef& shader);
	};


	template <typename F>
	void TerrainComponent::processRenderableShaders(F callback) const
	{
		for (auto& shader : mShaders) {
			callback(shader);
		}
	}

}

#endif		// TERRAIN_COMPONENT_H
