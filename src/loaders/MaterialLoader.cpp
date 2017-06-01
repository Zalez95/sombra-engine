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
			throw std::runtime_error("Error parsing the Material in the file \"" + fileReader->getFilePath() + "\":\n" + e.what());
		}
	}

// Private functions
	bool MaterialLoader::checkHeader(FileReader* fileReader)
	{
		const std::string FILE_VERSION = std::to_string(FILE_FORMAT::VERSION) + '.' + std::to_string(FILE_FORMAT::REVISION);
		bool ret = false;

		std::string fileName, fileVersion;
		if (fileReader->getValue(fileName)
			&& fileReader->getValue(fileVersion)
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
			if (fileReader->getValue(token)) {
				if (token == "num_materials") {
					fileReader->getValue(numMaterials);
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
		fileReader->getValue(name);
		fileReader->getValue(trash);

		bool end = false;
		while (!end) {
			std::string token;
			fileReader->getValue(token);

			if (token == "ambient_color") {
				fileReader->getValue(ambientColor.r);
				fileReader->getValue(ambientColor.g);
				fileReader->getValue(ambientColor.b);
			}
			else if (token == "diffuse_color") {
				fileReader->getValue(diffuseColor.r);
				fileReader->getValue(diffuseColor.g);
				fileReader->getValue(diffuseColor.b);
			}
			else if (token == "specular_color") {
				fileReader->getValue(specularColor.r);
				fileReader->getValue(specularColor.g);
				fileReader->getValue(specularColor.b);
			}
			else if (token == "specular_shininess") {
				fileReader->getValue(shininess);
			}
			else if (token == "}") { end = true; }
			else {
				throw std::runtime_error("Error: unexpected word \"" + token + "\" at line "+ std::to_string(fileReader->getNumLines()) + '\n');
			}
		}

		return std::make_unique<Material>(name, ambientColor, diffuseColor, specularColor, shininess);
	}

}
