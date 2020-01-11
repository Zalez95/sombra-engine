#ifndef FONT_H
#define FONT_H

#include <memory>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace se::graphics {

	class Texture;


	/**
	 * Struct Character, it represents a Character in ASCII code used to
	 * render text with OpenGL.
	 * It stores all the coordinates and data used to render the character
	 */
	struct Character
	{
		/** The texture of the Character glyph */
		std::unique_ptr<Texture> texture;

		/** The size in pixels of the Character glyph */
		glm::uvec2 size;

		/** The offset distance of top left corner of the Character in relation
		 * to the cursor position */
		glm::uvec2 offset;

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

		/** The maps each character with its actual graphics representation */
		std::unordered_map<char, Character> characters;
	};

}

#endif		// FONT_H
