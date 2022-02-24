#ifndef FONT_READER_H
#define FONT_READER_H

#include <vector>
#include "Result.h"
#include "../../graphics/2D/Font.h"

namespace se::app {

	/**
	 * Class FontReader, it's used for loading the Fonts from the given
	 * files
	 */
	class FontReader
	{
	private:	// Attributes
		/** The separation in pixels between glyphs in the font atlas */
		static constexpr unsigned int kGlyphSeparation = 2;

	public:		// Functions
		/** Reads the font located at the given path
		 *
		 * @param	context the graphics Context used for creating the Font
		 *			Bindables
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
		static Result read(
			graphics::Context& context,
			const char* path,
			const std::vector<char>& characterSet,
			const glm::uvec2& characterSize,
			const glm::uvec2& atlasSize, graphics::Font& output
		);
	};

}

#endif		// FONT_READER_H
