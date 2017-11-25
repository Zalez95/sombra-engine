#ifndef TERRAIN_READER_H
#define TERRAIN_READER_H

#include <memory>
#include "ImageReader.h"
#include "TerrainLoader.h"

namespace utils { class FileReader; }

namespace loaders {

	/**
	 * Class TerrainReader, it's used to create meshes from raw data or from the
	 * given files
	 */
	class TerrainReader
	{
	private:	// Nested types
		typedef std::unique_ptr<game::Entity> EntityUPtr;

	private:	// Attributes
		/** The TerrainLoader that we will use to create the Terrains */
		TerrainLoader& mTerrainLoader;

		/** The ImageReader used to read the height maps and textures of the
		 * Terrains */
		ImageReader& mImageReader;

	public:		// Functions
		/** Creates a new TerrainReader
		 * 
		 * @param	terrainLoader a reference to the TerrainLoader that we will
		 *			use to create the Terrains
		 * @param	imageReader a reference to the ImageReader the that we will
		 *			use to create the Terrains */
		TerrainReader(
			TerrainLoader& terrainLoader,
			ImageReader& imageReader
		) : mTerrainLoader(terrainLoader), mImageReader(imageReader) {};

		/** Class destructor */
		~TerrainReader() {};

		/** Parses the Terrain in the given file and returns it
		 * 
		 * @note	the cursor of the file reader will be moved after parsing
		 *			the file
		 * @param	fileReader the file reader with the Terrain that we want
		 *			to parse
		 * @return	the parsed Terrain */
		EntityUPtr read(utils::FileReader& fileReader);
	private:
		/** Parses the Entity at the current position of the given file and
		 * returns it
		 *
		 * @param	fileReader the file reader with the file that we want
		 *			to read
		 * @return	a pointer to the parsed Entity */
		EntityUPtr parseEntity(utils::FileReader& fileReader);
	};

}

#endif		// TERRAIN_READER_H
