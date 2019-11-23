#ifndef TERRAIN_LOADER_H
#define TERRAIN_LOADER_H

#include <memory>
#include <string>
#include "../GraphicsManager.h"
#include "../PhysicsManager.h"
#include "../CollisionManager.h"

namespace se::graphics { class Mesh; }
namespace se::collision { class TerrainCollider; }

namespace se::app {

	struct Entity;
	struct Image;
	struct RawMesh;


	/**
	 * Class TerrainLoader, it's used to create terrain meshes from raw data
	 */
	class TerrainLoader
	{
	private:	// Nested types
		using EntityUPtr = std::unique_ptr<app::Entity>;
		using TerrainColliderUPtr = std::unique_ptr<collision::TerrainCollider>;
		using RawMeshUPtr = std::unique_ptr<RawMesh>;

	private:	// Attributes
		/** The maximum color value that a pixel can have in the heightMaps */
		static constexpr unsigned char kMaxColor = 255;

		/** The GraphicsManager used to store the Renderable3Ds of the
		 * Terrains */
		app::GraphicsManager& mGraphicsManager;

		/** The PhysicsManager used to store the PhysicsEntities of the
		 * Terrains */
		app::PhysicsManager& mPhysicsManager;

		/** The CollisionManager used to store the Colliders of the Terrains */
		app::CollisionManager& mCollisionManager;

	public:		// Functions
		/** Creates a new TerrainLoader
		 *
		 * @param	graphicsManager the GraphicsManager that holds the graphics
		 *			data of the Terrain Entities
		 * @param	physicsManager the PhysicsManager that holds the physics
		 *			data of the Terrain Entities
		 * @param	collisionManager the CollisionManager that holds the
		 *			collider data of the Terrain Entities */
		TerrainLoader(
			app::GraphicsManager& graphicsManager,
			app::PhysicsManager& physicsManager,
			app::CollisionManager& collisionManager
		) : mGraphicsManager(graphicsManager),
			mPhysicsManager(physicsManager),
			mCollisionManager(collisionManager) {};

		/** Creates an Entity that represents a Terrain from the given data
		 *
		 * @param	name the name of the new Entity
		 * @param	size the length in the X and Z axis of the Terrain
		 * @param	heightMap an Image used that contains the height of the
		 * 			vertices of the Terrain's mesh
		 * @param	maxHeight the maximum height of the vertices of the
		 *			Terrain's mesh
		 * @return	a pointer to the new Terrain entity */
		EntityUPtr createTerrain(
			const std::string& name, float size,
			const Image& heightMap, float maxHeight
		);
	private:
		/** Creates the mesh of the Terrain from the given data
		 *
		 * @param	name the name of the mesh
		 * @param	heightMap an Image that represents the height map
		 *			that contains the positions in the Y axis of the vertices
		 *			of the Terrain
		 * @return	a pointer to the new RawMesh */
		static RawMeshUPtr createRawMesh(
			const std::string& name, const Image& heightMap
		);

		/** Creates a new TerrainCollider from the given height map
		 *
		 * @param	heightMap an Image that represents the height map
		 *			that contains the positions in the Y axis of the vertices
		 *			of the Terrain.
		 * @return	a pointer to the new TerrainCollider */
		static TerrainColliderUPtr createTerrainCollider(
			const Image& heightMap
		);

		/** Return the height located in the height map at the given position
		 *
		 * @param	heightMap an Image that represents the height map
		 *			that contains the positions in the Y axis of the vertices
		 *			of the Terrain.
		 * @param	x the position in the X axis that we want to read
		 * @param	z the position in the Z axis that we want to read
		 * @return	the height at the given position in the range [-0.5, 0.5] */
		static float getHeight(
			const Image& heightMap, std::size_t x, std::size_t z
		);
	};

}

#endif		// TERRAIN_LOADER_H
