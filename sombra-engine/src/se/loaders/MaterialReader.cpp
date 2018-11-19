#include <string>
#include <sstream>
#include "se/loaders/MaterialReader.h"
#include "se/utils/FileReader.h"
#include "se/graphics/3D/Material.h"

namespace se::loaders {

// Static variables definition
	const std::string MaterialReader::FileFormat::sFileName			= "SOMBRA_MAT_FILE";
	const std::string MaterialReader::FileFormat::sFileExtension	= ".semat";

// Public functions
	std::vector<MaterialReader::MaterialUPtr> MaterialReader::read(utils::FileReader& fileReader) const
	{
		try {
			// 1. Get the input file
			if (fileReader.getState() != utils::FileState::OK) {
				throw std::runtime_error("Error reading the file");
			}

			// 2. Check the file header
			if (!checkHeader(fileReader)) {
				throw std::runtime_error("Error with the header of the file");
			}

			// 3. Parse the Meshes
			return parseMaterials(fileReader);
		}
		catch (const std::exception& e) {
			throw std::runtime_error("Error parsing the Material in the file \"" + fileReader.getFilePath() + "\":\n" + e.what());
		}
	}

// Private functions
	bool MaterialReader::checkHeader(utils::FileReader& fileReader) const
	{
		const std::string curFileVersion = std::to_string(FileFormat::sVersion) + '.' + std::to_string(FileFormat::sRevision);
		bool ret = false;

		std::string fileName, fileVersion;
		fileReader >> fileName >> fileVersion;
		if ((fileReader.getState() == utils::FileState::OK)
			&& (fileName == FileFormat::sFileName)
			&& (fileVersion == curFileVersion)
		) {
			ret = true;
		}

		return ret;
	}


	std::vector<MaterialReader::MaterialUPtr> MaterialReader::parseMaterials(utils::FileReader& fileReader) const
	{
		std::vector<MaterialUPtr> materials;
		unsigned int nMaterials = 0, iMaterial = 0;

		std::string token;
		while (fileReader.getValue(token) == utils::FileState::OK) {
			if (token == "num_materials") {
				if (fileReader.getValue(nMaterials) == utils::FileState::OK) {
					materials.reserve(nMaterials);
				}
			}
			else if (token == "material") {
				auto curMaterial = parseMaterial(fileReader);
				if (iMaterial < nMaterials) {
					materials.push_back(std::move(curMaterial));
				}
				++iMaterial;
			}
			else {
				throw std::runtime_error("Error: unexpected word \"" + token + "\" at line " + std::to_string(fileReader.getNumLines()));
			}
		}

		if (iMaterial != nMaterials) {
			throw std::runtime_error("Error: expected " + std::to_string(nMaterials) + " materials, parsed " + std::to_string(iMaterial));
		}

		return materials;
	}


	MaterialReader::MaterialUPtr MaterialReader::parseMaterial(utils::FileReader& fileReader) const
	{
		std::string name;
		glm::vec3 ambientColor, diffuseColor, specularColor;
		float shininess;

		std::string trash;
		fileReader >> name >> trash;

		bool end = false;
		while (!end) {
			std::string token; fileReader >> token;

			if (token == "ambient_color") {
				fileReader >> ambientColor.r >> ambientColor.g >> ambientColor.b;
			}
			else if (token == "diffuse_color") {
				fileReader >> diffuseColor.r >> diffuseColor.g >> diffuseColor.b;
			}
			else if (token == "specular_color") {
				fileReader >> specularColor.r >> specularColor.g >> specularColor.b;
			}
			else if (token == "specular_shininess") {
				fileReader >> shininess;
			}
			else if (token == "}") { end = true; }
			else {
				throw std::runtime_error("Error: unexpected word \"" + token + "\" at line "+ std::to_string(fileReader.getNumLines()));
			}
		}

		return std::make_unique<graphics::Material>(name, ambientColor, diffuseColor, specularColor, shininess);
	}

}
