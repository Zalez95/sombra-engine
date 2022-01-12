#ifndef FONT_H
#define FONT_H

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include "../Context.h"

namespace se::graphics {

	class Texture;


	/**
	 * Struct Character, it represents a Character in ASCII code used for
	 * rendering text with OpenGL.
	 * It stores all the coordinates and data used for rendering the characters
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
		/** The name of the font */
		std::string name;

		/** The atlas texture with the character glyphs */
		Context::TBindableRef<Texture> textureAtlas;

		/** The size of the texture atlas */
		glm::uvec2 atlasSize;

		/** The maps each character with its actual graphics representation */
		std::unordered_map<char, Character> characters;

		/** The maximum size in pixels that any character can have */
		glm::uvec2 maxCharacterSize;
	};

}

#endif		// FONT_H
