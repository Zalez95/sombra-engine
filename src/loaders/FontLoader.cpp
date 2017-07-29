#include "FontLoader.h"
#include "../utils/FileReader.h"
#include "../graphics/text/Font.h"
#include "../graphics/Texture.h"

namespace loaders {

	FontLoader::FontUPtr FontLoader::load(utils::FileReader& fileReader) const
	{
		try {
			// 1. Get the input file
			if (fileReader.fail()) {
				throw std::runtime_error("Error reading the file\n");
			}

			// 2. Parse the Meshes
			return parseFont(fileReader);
		}
		catch (const std::exception& e) {
			throw std::runtime_error("Error parsing the Font in the file \"" + fileReader.getFilePath() + "\":\n" + e.what());
		}
	}

// Private functions
	FontLoader::FontUPtr FontLoader::parseFont(utils::FileReader& fileReader) const
	{
		std::string fontName, fontTextureName, trash;
		std::vector<graphics::Character> characters;
		unsigned int numCharacters = 0, characterIndex = 0;

		while (!fileReader.isEmpty()) {
			std::string token; fileReader >> token;

			if (token == "info") {
				fileReader.getValuePair(trash, "=", fontName);
				fileReader.discardLine();
			}
			else if (token == "common") {
				fileReader.discardLine();
			}
			else if (token == "page") {
				fileReader >> trash;
				fileReader.getValuePair(trash, "=", fontTextureName);
			}
			else if (token == "chars") {
				fileReader.getValuePair(trash, "=", numCharacters);
				if (!fileReader.fail()) {
					characters.reserve(numCharacters);
				}
			}
			else if (token == "char") {
				graphics::Character curChar = parseCharacter(fileReader);
				if (characterIndex < numCharacters) {
					characters.push_back(curChar);
				}
				++characterIndex;
			}
			else {
				throw std::runtime_error("Error: unexpected word \"" + token + "\" at line " + std::to_string(fileReader.getNumLines()) + '\n');
			}
		}

		if (characterIndex != numCharacters) {
			throw std::runtime_error("Error: expected " + std::to_string(numCharacters) + " characters, parsed " + std::to_string(characterIndex) + '\n');
		}

		auto textureAtlas = std::make_shared<graphics::Texture>(fileReader.getDirectory() + fontTextureName, GL_TEXTURE_2D);
		return std::make_unique<graphics::Font>(fontName, characters, textureAtlas);
	}


	graphics::Character FontLoader::parseCharacter(utils::FileReader& fileReader) const
	{
		graphics::Character ret;
		std::string name, trash;
		int intValue;

		bool flag = true;
		while (flag) {
			fileReader.getValuePair(name, "=", intValue);
			if (name == "id") {
				ret.mId = intValue;
			}
			else if (name == "x") {
				ret.mPosition.x = intValue;
			}
			else if (name == "y") {
				ret.mPosition.y = intValue;
			}
			else if (name == "width") {
				ret.mSize.x = intValue;
			}
			else if (name == "height") {
				ret.mSize.y = intValue;
			}
			else if (name == "xoffset") {
				ret.mOffset.x = intValue;
			}
			else if (name == "yoffset") {
				ret.mOffset.y = intValue;
			}
			else if (name == "xadvance") {
				ret.mAdvance = intValue;
			}
			else if (name == "page") {
				continue;
			}
			else if (name == "chnl") {
				flag = false;
			}
			else {
				throw std::runtime_error("Error: unexpected word \"" + name + "\" at line " + std::to_string(fileReader.getNumLines()) + '\n');
			}
		}

		return ret;
	}

}
