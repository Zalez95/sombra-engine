#include "EntityReader.h"
#include <string>
#include <sstream>
#include "../utils/FileReader.h"
#include "../game/Entity.h"
#include "../graphics/3D/Renderable3D.h"
#include "../graphics/3D/Mesh.h"
#include "../graphics/3D/Material.h"

namespace loaders {

// Nested types definition
	struct EntityReader::SharedData
	{
		std::vector<std::shared_ptr<graphics::Mesh>> mMeshes;
		std::vector<std::shared_ptr<graphics::Material>> mMaterials;
	};

// Static variables definition
	const std::string EntityReader::FILE_FORMAT::FILE_NAME = "FAZE_ENT_FILE";
	const std::string EntityReader::FILE_FORMAT::FILE_EXTENSION = ".fzent";

// Public Functions
	std::vector<EntityReader::EntityUPtr> EntityReader::load(utils::FileReader& fileReader)
	{
		try {
			// 1. Get the input file
			if (fileReader.getState() != utils::FileState::OK) {
				throw std::runtime_error("Error reading the file\n");
			}

			// 2. Check the file header
			if (!checkHeader(fileReader)) {
				throw std::runtime_error("Error with the header of the file\n");
			}

			// 3. Load the SharedData
			SharedData sharedData = loadSharedData(fileReader);

			// 4. Parse the Entities
			return parseEntities(fileReader, sharedData);
		}
		catch (const std::exception& e) {
			throw std::runtime_error("Error parsing the Entity in the file \"" + fileReader.getFilePath() + "\":\n" + e.what());
		}
	}

// Private functions
	bool EntityReader::checkHeader(utils::FileReader& fileReader) const
	{
		const std::string FILE_VERSION = std::to_string(FILE_FORMAT::VERSION) + '.' + std::to_string(FILE_FORMAT::REVISION);
		bool ret = false;

		std::string fileName, fileVersion;
		fileReader >> fileName >> fileVersion;
		if ((fileReader.getState() == utils::FileState::OK) &&
			(fileName == FILE_FORMAT::FILE_NAME) &&
			(fileVersion == FILE_VERSION)
		) {
			ret = true;
		}

		return ret;
	}


	EntityReader::SharedData EntityReader::loadSharedData(utils::FileReader& fileReader) const
	{
		SharedData ret;
		std::string trash;

		bool end = false;
		while (!end) {
			std::string token; fileReader >> token;

			if (token == "files") {
				fileReader >> trash;
			}
			else if (token == "mesh") {
				fileReader >> token;
				utils::FileReader meshFileReader(token);
				auto meshes = mMeshReader.load(meshFileReader);
				ret.mMeshes.reserve(meshes.size());
				ret.mMeshes.insert(
					ret.mMeshes.end(),
					std::make_move_iterator(meshes.begin()),
					std::make_move_iterator(meshes.end())
				);
			}
			else if (token == "material") {
				fileReader >> token;
				utils::FileReader materialFileReader(token);
				auto materials = mMaterialReader.load(materialFileReader);
				ret.mMaterials.reserve(materials.size());
				ret.mMaterials.insert(
					ret.mMaterials.end(),
					std::make_move_iterator(materials.begin()),
					std::make_move_iterator(materials.end())
				);
			}
			else if (token == "}") { end = true; }
			else {
				throw std::runtime_error("Error: unexpected word \"" + token + "\" at line " + std::to_string(fileReader.getNumLines()) + '\n');
			}
		}

		return ret;
	}

	
	std::vector<EntityReader::EntityUPtr> EntityReader::parseEntities(
		utils::FileReader& fileReader, SharedData& sharedData
	) const
	{
		std::vector<EntityUPtr> entities;
		unsigned int numEntities = 0, entityIndex = 0;

		std::string token;
		while (fileReader.getValue(token) == utils::FileState::OK) {
			if (token == "num_entities") {
				if (fileReader.getValue(numEntities) == utils::FileState::OK) {
					entities.reserve(numEntities);
				}
			}
			else if (token == "entity") {
				auto curEntity = parseEntity(fileReader, sharedData);
				if (entityIndex < numEntities) {
					entities.push_back(std::move(curEntity));
				}
				++entityIndex;
			}
			else {
				throw std::runtime_error("Error: unexpected word \"" + token + "\" at line " + std::to_string(fileReader.getNumLines()) + '\n');
			}
		}

		if (entityIndex != numEntities) {
			throw std::runtime_error("Error: expected " + std::to_string(numEntities) + " entities, parsed " + std::to_string(entityIndex) + '\n');
		}

		return entities;
	}

	EntityReader::EntityUPtr EntityReader::parseEntity(
		utils::FileReader& fileReader, SharedData& /* sharedData */
	) const
	{
		std::string name;
		glm::vec3 position;
		glm::quat orientation;

		std::string trash;
		fileReader >> name >> trash;

		std::string token;
		while (fileReader.getValue(token) == utils::FileState::OK) {
			if (token == "position") {
				fileReader >> position.x >> position.y >> position.z;
			}
			else if (token == "orientation") {
				fileReader >> orientation.w >> orientation.x >> orientation.y >> orientation.z;
			}
			else if (token == "camera") {
				glm::vec3 cameraPosition, cameraTarget, cameraUp;
				fileReader >> cameraPosition.x	>> cameraPosition.y	>> cameraPosition.z;
				fileReader >> cameraTarget.x	>> cameraTarget.y	>> cameraTarget.z;
				fileReader >> cameraUp.x		>> cameraUp.y		>> cameraUp.z;

				auto camera = std::make_unique<graphics::Camera>(cameraPosition, cameraTarget, cameraUp);
			}
			else if (token == "renderable3D") {

			}
			else if (token == "point_light") {
				
			}
			else {
				throw std::runtime_error("Error: unexpected word \"" + token + "\" at line " + std::to_string(fileReader.getNumLines()) + '\n');
			}
		}

		auto entity = std::make_unique<game::Entity>(name);
		entity->mPosition		= position;
		entity->mOrientation	= orientation;
		return std::move(entity);
	}

}
