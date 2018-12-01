#include "se/loaders/FontReader.h"
#include "se/loaders/ImageReader.h"
#include "se/utils/Image.h"
#include "se/utils/FileReader.h"
#include "se/graphics/text/Font.h"
#include "se/graphics/Texture.h"

namespace se::loaders {

	FontReader::FontUPtr FontReader::read(utils::FileReader& fileReader) const
	{
		try {
			// 1. Get the input file
			if (fileReader.getState() != utils::FileState::OK) {
				throw std::runtime_error("Error reading the file");
			}

			// 2. Parse the Meshes
			return parseFont(fileReader);
		}
		catch (const std::exception& e) {
			throw std::runtime_error("Error parsing the Font in the file \"" + fileReader.getFilePath() + "\": " + e.what());
		}
	}

// Private functions
	FontReader::FontUPtr FontReader::parseFont(utils::FileReader& fileReader) const
	{
		std::string fontName, trash;
		std::vector<graphics::Character> characters;
		auto textureAtlas = std::make_shared<graphics::Texture>();
		unsigned int nCharacters = 0, iCharacter = 0;

		std::string token;
		while (fileReader.getValue(token) == utils::FileState::OK) {
			if (token == "info") {
				fileReader.getValuePair(trash, "=", fontName);
				fileReader.discardLine();
			}
			else if (token == "common") {
				fileReader.discardLine();
			}
			else if (token == "page") {
				fileReader >> trash;
				std::string fontTextureName;
				fileReader.getValuePair(trash, "=", fontTextureName);
				std::string fontTexturePath = fileReader.getDirectory() + fontTextureName.substr(1, fontTextureName.size() - 2);

				utils::Image atlasImg = ImageReader::read(fontTexturePath, utils::ImageFormat::L_IMAGE);
				textureAtlas->setImage(
					atlasImg.getPixels(), graphics::TexturePixelType::BYTE, graphics::TextureFormat::RED,
					atlasImg.getWidth(), atlasImg.getHeight()
				);
			}
			else if (token == "chars") {
				if (fileReader.getValuePair(trash, "=", nCharacters) == utils::FileState::OK) {
					characters.reserve(nCharacters);
				}
			}
			else if (token == "char") {
				if (iCharacter < nCharacters) {
					characters.push_back(parseCharacter(fileReader));
				}
				++iCharacter;
			}
			else {
				throw std::runtime_error("Error: unexpected word \"" + token + "\" at line " + std::to_string(fileReader.getNumLines()));
			}
		}

		if (iCharacter != nCharacters) {
			throw std::runtime_error("Error: expected " + std::to_string(nCharacters) + " characters, parsed " + std::to_string(iCharacter));
		}

		return std::make_unique<graphics::Font>(fontName, characters, textureAtlas);
	}


	graphics::Character FontReader::parseCharacter(utils::FileReader& fileReader) const
	{
		graphics::Character ret;
		std::string name, trash;
		int intValue;

		bool flag = true;
		while (flag) {
			fileReader.getValuePair(name, "=", intValue);
			if (name == "id") {
				ret.id = intValue;
			}
			else if (name == "x") {
				ret.position.x = intValue;
			}
			else if (name == "y") {
				ret.position.y = intValue;
			}
			else if (name == "width") {
				ret.size.x = intValue;
			}
			else if (name == "height") {
				ret.size.y = intValue;
			}
			else if (name == "xoffset") {
				ret.offset.x = intValue;
			}
			else if (name == "yoffset") {
				ret.offset.y = intValue;
			}
			else if (name == "xadvance") {
				ret.advance = intValue;
			}
			else if (name == "page") {
				continue;
			}
			else if (name == "chnl") {
				flag = false;
			}
			else {
				throw std::runtime_error("Error: unexpected word \"" + name + "\" at line " + std::to_string(fileReader.getNumLines()));
			}
		}

		return ret;
	}

}
