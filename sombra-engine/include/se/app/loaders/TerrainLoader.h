#ifndef TERRAIN_LOADER_H
#define TERRAIN_LOADER_H

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "../Entity.h"
#include "../graphics/Image.h"

namespace se::utils { class Repository; }
namespace se::graphics { class GraphicsEngine; class RenderableTerrain; }
namespace se::collision { class Collider; }

namespace se::app {

	class EntityDatabase;
	class CameraSystem;


	/**
	 * Class TerrainLoader, it's used for creating terrain meshes from raw data
	 */
	class TerrainLoader
	{
	private:	// Nested types
		using ColliderUPtr = std::unique_ptr<collision::Collider>;

	private:	// Attributes
		/** The maximum color value that a pixel can have in the heightMaps */
		static constexpr unsigned char kMaxColor = 255;

		/** The EntityDatabase that holds the Terrain Entities and Components */
		EntityDatabase& mEntityDatabase;

		/** The GraphicsEngine used for rendering the Terrain */
		graphics::GraphicsEngine& mGraphicsEngine;

		/** The CameraSystem that holds the passes */
		CameraSystem& mCameraSystem;

		/** The Repository that holds the graphics data */
		utils::Repository& mRepository;

	public:		// Functions
		/** Creates a new TerrainLoader
		 *
		 * @param	entityDatabase the EntityDatabase that holds the Terrain
		 *			Entities and Components
		 * @param	graphicsEngine the GraphicsEngine used for rendering the Terrain
		 * @param	cameraSystem the CameraSystem that holds the passes
		 * @param	repository the Repository that holds the graphics data */
		TerrainLoader(
			EntityDatabase& entityDatabase,
			graphics::GraphicsEngine& graphicsEngine,
			CameraSystem& cameraSystem, utils::Repository& repository
		) : mEntityDatabase(entityDatabase), mGraphicsEngine(graphicsEngine),
			mCameraSystem(cameraSystem), mRepository(repository) {};

		/** Creates an Entity that represents a Terrain from the given data
		 *
		 * @param	name the name of the new Entity
		 * @param	size the length in the X and Z axis of the Terrain
		 * @param	maxHeight the maximum height of the vertices of the
		 *			Terrain's mesh
		 * @param	heightMap an Image used that contains the height of the
		 * 			vertices of the Terrain's mesh
		 * @param	lodDistances the minimum distance to the camera at each
		 *			level of detail
		 * @param	techniqueName the name of the program used for rendering the
		 *			terrain
		 * @param	programName the name of the program used for the terrain
		 *			technique
		 * @return	the new Terrain entity */
		Entity createTerrain(
			const char* name, float size, float maxHeight,
			const Image<unsigned char>& heightMap,
			const std::vector<float>& lodDistances,
			const char* techniqueName, const char* programName
		);
	private:
		/** Creates a new RenderableTerrain from the given data
		 *
		 * @param	size the length in the X and Z axis of the Terrain
		 * @param	maxHeight the maximum height of the vertices of the
		 *			Terrain's mesh
		 * @param	heightMap an Image used that contains the height of the
		 * 			vertices of the Terrain's mesh
		 * @param	lodDistances the minimum distance to the camera at each
		 *			level of detail
		 * @param	techniqueName the name of the program used for rendering the
		 *			terrain
		 * @param	programName the name of the program used for the terrain
		 *			technique
		 * @return	a pointer to the new RenderableTerrain, nullptr if it
		 *			failed */
		graphics::RenderableTerrain createTerrainRenderable(
			float size, float maxHeight,
			const Image<unsigned char>& heightMap,
			const std::vector<float>& lodDistances,
			const char* techniqueName, const char* programName
		);

		/** Creates a new TerrainCollider from the given height map
		 *
		 * @param	heightMap an Image that represents the height map
		 *			that contains the positions in the Y axis of the vertices
		 *			of the Terrain.
		 * @param	scaleVector a vector with the scale of the TerrainCollider
		 *			in each axis
		 * @return	a pointer to the new Collider */
		static ColliderUPtr createTerrainCollider(
			const Image<unsigned char>& heightMap, const glm::vec3& scaleVector
		);

		/** Return the height located in the height map at the given position
		 *
		 * @param	heightMap an Image that represents the height map
		 *			that contains the positions in the Y axis of the vertices
		 *			of the Terrain
		 * @param	x the position in the X axis that we want to read
		 * @param	z the position in the Z axis that we want to read
		 * @return	the height at the given position in the range [-0.5, 0.5] */
		static float getHeight(
			const Image<unsigned char>& heightMap, std::size_t x, std::size_t z
		);
	};

}

#endif		// TERRAIN_LOADER_H
