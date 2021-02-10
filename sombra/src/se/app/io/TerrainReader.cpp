#include "se/app/io/TerrainReader.h"
#include "se/app/io/ImageReader.h"
#include "se/utils/FileReader.h"

namespace se::app {

	Entity TerrainReader::read(utils::FileReader& fileReader)
	{
		try {
			// 1. Get the input file
			if (fileReader.getState() != utils::FileState::OK) {
				throw std::runtime_error("Error reading the file");
			}

			// 2. Parse the Entity
			return parseEntity(fileReader);
		}
		catch (const std::exception& e) {
			throw std::runtime_error("Error parsing the Terrain in the file \"" + fileReader.getFilePath() + "\": " + e.what());
		}
	}

// Private functions
	Entity TerrainReader::parseEntity(utils::FileReader& fileReader)
	{
		std::string name, heightMapPath;
		float size = 0.0f, maxHeight = 0.0f;

		std::string trash;
		fileReader >> name >> trash;

		bool end = false;
		while (!end) {
			std::string token; fileReader >> token;

			if (token == "size") {
				fileReader >> size;
			}
			else if (token == "height_map") {
				fileReader >> heightMapPath;
			}
			else if (token == "max_height") {
				fileReader >> maxHeight;
			}
			else if (token == "}") { end = true; }
			else {
				throw std::runtime_error("Error: unexpected word \"" + token + "\" at line "+ std::to_string(fileReader.getNumLines()));
			}
		}

		Image<unsigned char> heightMap;
		Result result = ImageReader::read(heightMapPath.c_str(), heightMap, 1);
		if (!result) {
			throw std::runtime_error("Error while reading the heighMap \"" + heightMapPath + "\": " + result.description());
		}

		return mTerrainLoader.createTerrain(name.c_str(), size, maxHeight, heightMap, {}, nullptr);
	}

}
