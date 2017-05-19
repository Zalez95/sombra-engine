#include "MaterialLoader.h"
#include <string>
#include <sstream>
#include "../utils/FileReader.h"
#include "../graphics/3D/Material.h"

namespace graphics{

// Static variables definition
	const std::string MaterialLoader::FILE_FORMAT::FILE_NAME				= "FAZE_MAT_FILE";
	const std::string MaterialLoader::FILE_FORMAT::MATERIAL_FILE_EXTENSION	= ".fzmat";

// Public functions
	std::vector<MaterialLoader::MaterialUPtr> MaterialLoader::load(FileReader* fileReader)
	{
		try {
			// 1. Get the input file
			if (!fileReader || fileReader->fail()) {
				throw std::runtime_error("Error reading the file\n");
			}

			// 2. Check the file header
			if (!checkHeader(fileReader)) {
				throw std::runtime_error("Error with the header of the file\n");
			}

			// 3. Parse the Meshes
			return parseMaterials(fileReader);
		}
		catch (const std::exception& e) {
			throw std::runtime_error("Error parsing the Material in the file \"" + fileReader->getCurrentFilePath() + "\":\n" + e.what());
		}
	}

// Private functions
	bool MaterialLoader::checkHeader(FileReader* fileReader)
	{
		const std::string FILE_VERSION = std::to_string(FILE_FORMAT::VERSION) + '.' + std::to_string(FILE_FORMAT::REVISION);
		bool ret = false;

		std::string fileName, fileVersion;
		if (fileReader->getParam(fileName)
			&& fileReader->getParam(fileVersion)
			&& fileName == FILE_FORMAT::FILE_NAME
			&& fileVersion == FILE_VERSION)
		{
			ret = true;
		}

		return ret;
	}

	
	std::vector<MaterialLoader::MaterialUPtr> MaterialLoader::parseMaterials(FileReader* fileReader)
	{
		std::vector<MaterialUPtr> materials;
		unsigned int numMaterials = 0, materialIndex = 0;

		while (!fileReader->eof()) {
			std::string token;
			if (fileReader->getParam(token)) {
				if (token == "num_materials") {
					fileReader->getParam(numMaterials);
					materials.resize(numMaterials);
				}
				else if (token == "material") {
					if (materialIndex < numMaterials) {
						materials[materialIndex] = parseMaterial(fileReader);
					}
					++materialIndex;
				}
				else {
					throw std::runtime_error("Error: unexpected word \"" + token + "\" at line " + std::to_string(fileReader->getNumLines()) + '\n');
				}
			}
		}

		if (materialIndex != numMaterials) {
			throw std::runtime_error("Error: expected " + std::to_string(numMaterials) + " materials, parsed " + std::to_string(materialIndex) + '\n');
		}

		return materials;
	}


	MaterialLoader::MaterialUPtr MaterialLoader::parseMaterial(FileReader* fileReader)
	{
		std::string name;
		RGBColor ambientColor, diffuseColor, specularColor;
		float shininess;
		
		std::string trash;
		fileReader->getParam(name);
		fileReader->getParam(trash);

		bool end = false;
		while (!end) {
			std::string token;
			fileReader->getParam(token);

			if (token == "ambient_color") {
				fileReader->getParam(ambientColor.r);
				fileReader->getParam(ambientColor.g);
				fileReader->getParam(ambientColor.b);
			}
			else if (token == "diffuse_color") {
				fileReader->getParam(diffuseColor.r);
				fileReader->getParam(diffuseColor.g);
				fileReader->getParam(diffuseColor.b);
			}
			else if (token == "specular_color") {
				fileReader->getParam(specularColor.r);
				fileReader->getParam(specularColor.g);
				fileReader->getParam(specularColor.b);
			}
			else if (token == "specular_shininess") {
				fileReader->getParam(shininess);
			}
			else if (token == "}") { end = true; }
			else {
				throw std::runtime_error("Error: unexpected word \"" + token + "\" at line "+ std::to_string(fileReader->getNumLines()) + '\n');
			}
		}

		return std::make_unique<Material>(name, ambientColor, diffuseColor, specularColor, shininess);
	}

}
