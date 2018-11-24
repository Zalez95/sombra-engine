#ifndef TERRAIN_READER_H
#define TERRAIN_READER_H

#include <memory>
#include "TerrainLoader.h"

namespace se::utils { class FileReader; }

namespace se::loaders {

	/**
	 * Class TerrainReader, it's used to create meshes from raw data or from the
	 * given files
	 */
	class TerrainReader
	{
	private:	// Nested types
		using EntityUPtr = std::unique_ptr<app::Entity>;

	private:	// Attributes
		/** The TerrainLoader that we will use to create the Terrains */
		TerrainLoader& mTerrainLoader;

	public:		// Functions
		/** Creates a new TerrainReader
		 *
		 * @param	terrainLoader a reference to the TerrainLoader that we will
		 *			use to create the Terrains */
		TerrainReader(TerrainLoader& terrainLoader) :
			mTerrainLoader(terrainLoader) {};

		/** Parses the Terrain in the given file and returns it
		 *
		 * @note	the cursor of the file reader will be moved after parsing
		 *			the file
		 * @param	fileReader the file reader with the Terrain that we want
		 *			to parse
		 * @return	the parsed Terrain
		 * @throw	runtime_error in case of any error while parsing */
		EntityUPtr read(utils::FileReader& fileReader);
	private:
		/** Parses the Entity at the current position of the given file and
		 * returns it
		 *
		 * @param	fileReader the file reader with the file that we want
		 *			to read
		 * @return	a pointer to the parsed Entity
		 * @throw	runtime_error in case of an unexpected text */
		EntityUPtr parseEntity(utils::FileReader& fileReader);
	};

}

#endif		// TERRAIN_READER_H
