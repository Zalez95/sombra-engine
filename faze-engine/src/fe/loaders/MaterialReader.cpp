#include "fe/loaders/MaterialReader.h"
#include <string>
#include <sstream>
#include "fe/utils/FileReader.h"
#include "fe/graphics/3D/Material.h"

namespace fe { namespace loaders {

// Static variables definition
	const std::string MaterialReader::FileFormat::sFileName			= "FAZE_MAT_FILE";
	const std::string MaterialReader::FileFormat::sFileExtension	= ".fzmat";

// Public functions
	std::vector<MaterialReader::MaterialUPtr> MaterialReader::read(utils::FileReader& fileReader) const
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
				throw std::runtime_error("Error: unexpected word \"" + token + "\" at line " + std::to_string(fileReader.getNumLines()) + '\n');
			}
		}

		if (iMaterial != nMaterials) {
			throw std::runtime_error("Error: expected " + std::to_string(nMaterials) + " materials, parsed " + std::to_string(iMaterial) + '\n');
		}

		return materials;
	}


	MaterialReader::MaterialUPtr MaterialReader::parseMaterial(utils::FileReader& fileReader) const
	{
		std::string name;
		graphics::RGBColor ambientColor, diffuseColor, specularColor;
		float shininess;

		std::string trash;
		fileReader >> name >> trash;

		bool end = false;
		while (!end) {
			std::string token; fileReader >> token;

			if (token == "ambient_color") {
				fileReader >> ambientColor.mR >> ambientColor.mG >> ambientColor.mB;
			}
			else if (token == "diffuse_color") {
				fileReader >> diffuseColor.mR >> diffuseColor.mG >> diffuseColor.mB;
			}
			else if (token == "specular_color") {
				fileReader >> specularColor.mR >> specularColor.mG >> specularColor.mB;
			}
			else if (token == "specular_shininess") {
				fileReader >> shininess;
			}
			else if (token == "}") { end = true; }
			else {
				throw std::runtime_error("Error: unexpected word \"" + token + "\" at line "+ std::to_string(fileReader.getNumLines()) + '\n');
			}
		}

		return std::make_unique<graphics::Material>(name, ambientColor, diffuseColor, specularColor, shininess);
	}

}}
