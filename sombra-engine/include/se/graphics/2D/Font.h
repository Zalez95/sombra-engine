#ifndef FONT_H
#define FONT_H

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include "../core/Texture.h"

namespace se::graphics {

	/**
	 * Struct Character, it represents a Character in ASCII code used to
	 * render text with OpenGL.
	 * It stores all the coordinates and data used to render the character
	 */
	struct Character
	{
		/** The position of the top left corner of the Character in the Font
		 * atlas texture */
		glm::uvec2 atlasPosition;

		/** The size in pixels of the Character glyph */
		glm::uvec2 size;

		/** The offset distance of top left corner of the Character in relation
		 * to the cursor position */
		glm::ivec2 offset;

		/** The horizontal advance to the next Character after adding the
		 * current Character to a text */
		unsigned int advance;
	};


	/**
	 * Struct Font, holds all the characters that can be represented with the
	 * Font
	 */
	struct Font
	{
		using Repository = utils::Repository<Font, unsigned short>;

		/** The name of the font */
		std::string name;

		/** The atlas texture with the character glyphs */
		Texture::Repository::Reference textureAtlas;

		/** The size of the texture atlas */
		glm::uvec2 atlasSize;

		/** The maps each character with its actual graphics representation */
		std::unordered_map<char, Character> characters;

		/** The maximum size in pixels that any character can have */
		glm::uvec2 maxCharacterSize;
	};

}

#endif		// FONT_H
