#ifndef FONT_H
#define FONT_H

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace graphics {

	class Texture;


	/**
	 * Struct Character, it represents a Character in ASCII code used to
	 * render text with OpenGL.
	 * It stores all the coordinates and data used to render the character
	 * from the Texture Atlas of its corresponding Font
	 */
	struct Character
	{
        /** The character ASCII id */
        unsigned int mId;
        
		/** The position of the top left corner of the Character in the Font
		 * texture */
        glm::vec2 mPosition;

		/** The size in pixels of the Character */
        glm::vec2 mSize;

		/** The offset distance of top left corner of the Character in relation
		 * to the cursor position */
        glm::vec2 mOffset;

		/** The horizontal advance to the next Character after adding the
		 * current Character to a text */
        float mAdvance;

		/** Creates a new Character */
		Character() {};

		/** Class destructor */
		~Character() {};
	};


	/**
	 * Class Font
	 */
	class Font
	{
	private:	// Nested types
		typedef std::shared_ptr<Texture> TextureSPtr;

	private:	// Attributes
        /** The name of the font */
        std::string mName;

        /** The characters of the font */
        std::vector<Character> mCharacters;

		/** The texture atlas of the Font */
		const TextureSPtr mTextureAtlas;

	public:		// Functions
		/** Creates a new Font
         *
         * @param   name the name of the font
         * @param   characters a vector with the Characters of the Font
		 * @param	textureAtlas a pointer to the texture atlas of the Font */
		Font(
			const std::string& name,
		   	const std::vector<Character>& characters,
			const TextureSPtr textureAtlas
		) : mName(name), mCharacters(characters),
	   		mTextureAtlas(textureAtlas) {};

		/** Class destructor */
		~Font() {};

		/** @return	the name of the Font */
		std::string getName() const { return mName; };

		/** @return	a pointer to the texture atlas of the Font */
		const TextureSPtr getTextureAtlas() const { return mTextureAtlas; };
	};

}

#endif		// FONT_H
