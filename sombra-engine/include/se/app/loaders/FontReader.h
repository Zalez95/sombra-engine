#ifndef FONT_READER_H
#define FONT_READER_H

#include <vector>
#include "Result.h"
#include "../../graphics/GraphicsEngine.h"

namespace se::app {

	/**
	 * Class FontReader, it's used for loading the Fonts from the given
	 * files
	 */
	class FontReader
	{
	private:	// Nested types
		using FontRef = se::graphics::Font::Repository::Reference;

	private:	// Attributes
		/** The separation in pixels between glyphs in the font atlas */
		static constexpr unsigned int kGlyphSeparation = 2;

		/** The GraphicsEngine used for storing Textures and Materials */
		graphics::GraphicsEngine& mGraphicsEngine;

	public:		// Functions
		/** Creates a new FontReader
		 *
		 * @param	graphicsEngine the GraphicsEngine used for storing
		 *			Textures and Materials */
		FontReader(graphics::GraphicsEngine& graphicsEngine) :
			mGraphicsEngine(graphicsEngine) {};

		/** Reads the font located at the given path
		 *
		 * @param	path the location of the Font
		 * @param	characterSet the set of characters to load to the Font. If
		 *			a character is not found in the Font it won't be loaded
		 *			(Only latin-1)
		 * @param	characterSize the nominal size of the loaded characters
		 * @param	atlasSize the size of the font atlas texture
		 * @param	output a Reference to the new Font (return parameter)
		 * @return	a Result object with the result of the operation
		 * @note	if the atlas texture isn't big enough to hold all the
		 *			characters some of them won't be added to the Font */
		Result read(
			const char* path,
			const std::vector<char>& characterSet, const glm::uvec2& characterSize,
			const glm::uvec2& atlasSize, FontRef& output
		);
	};

}

#endif		// FONT_READER_H
